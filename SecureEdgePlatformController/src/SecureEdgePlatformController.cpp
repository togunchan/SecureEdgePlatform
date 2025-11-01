#include <SecureEdgePlatformController.hpp>
#include <secureboot/BootStage.hpp>
#include <secureboot/BootSimulator.hpp>
#include <secureboot/BootConfig.hpp>
#include <filesystem>
#include <iostream>

namespace
{
    std::filesystem::path resolveBootConfigPath()
    {
        const std::filesystem::path sourceFile{__FILE__};
        const auto repoRoot = sourceFile.parent_path().parent_path().parent_path();
        return repoRoot / "SecureBootSimulator" / "data" / "boot_config.json";
    }
}

SecureEdgePlatformController::SecureEdgePlatformController() = default;
SecureEdgePlatformController::~SecureEdgePlatformController()
{
    stop();
}

void SecureEdgePlatformController::start()
{
    if (running_.exchange(true))
    {
        std::cout << "[Controller] Already running.\n";
        return;
    }

    std::cout << "[Controller] Starting SecureEdgePlatform..." << std::endl;

    if (!bootPhase())
    {
        std::cerr << "[Controller] Boot phase failed. Aborting startup.\n";
        running_.store(false);
        return;
    }
    loopThread_ = std::thread([this]()
                              { runLoop(); });
}

void SecureEdgePlatformController::stop()
{
    bool wasRunning = running_.exchange(false);
    if (wasRunning)
    {
        std::cout << "[Controller] Stopping SecureEdgePlatform..." << std::endl;
        gateway_.stopLoop();
    }

    if (loopThread_.joinable())
        loopThread_.join();
}

gateway::EdgeGateway &SecureEdgePlatformController::getGateway()
{
    return gateway_;
}

const gateway::EdgeGateway &SecureEdgePlatformController::getGateway() const
{
    return gateway_;
}

void SecureEdgePlatformController::runLoop()
{
    try
    {
        if (!running_.load(std::memory_order_acquire))
            return;

        gateway_.start("");
        gateway_.runLoop();
    }
    catch (const std::exception &ex)
    {
        std::cerr << "[Controller] Exception in runLoop: " << ex.what() << "\n";
    }
    catch (...)
    {
        std::cerr << "[Controller] Unknown exception in runLoop.\n";
    }
    running_.store(false, std::memory_order_release);
}

bool SecureEdgePlatformController::bootPhase()
{
    std::cout << "[BootPhase] Starting secure boot..." << std::endl;

    try
    {
        const std::filesystem::path configPath = resolveBootConfigPath();
        if (!std::filesystem::exists(configPath))
        {
            std::cerr << "[BootPhase] Config file not found: " << configPath << std::endl;
            return false;
        }
        secureboot::BootConfig config;
        if (!config.loadFromFile(configPath.string()))
        {
            std::cerr << "[BootPhase] Failed to load config from: " << configPath << std::endl;
            return false;
        }
        secureboot::BootSimulator simulator(config);
        simulator.addStage(secureboot::BootStage("Preflight Checks", 10, []()
                                                 { std::cout << "[BootStage] Preflight checks passed.\n"; }));
        simulator.addStage(secureboot::BootStage("Firmware Authentication", 20, []()
                                                 { std::cout << "[BootStage] Firmware authentication succeeded.\n"; }));
        simulator.addStage(secureboot::BootStage("Subsystem Bring-up", 30, []()
                                                 { std::cout << "[BootStage] Core subsystems online.\n"; }));
        simulator.run();

        if (!simulator.wasSuccessful())
        {
            std::cerr << "[BootPhase] Boot simulator reported failure: " << simulator.getFailureReason() << std::endl;
            return false;
        }

        std::cout << "[BootPhase] Boot successful. Proceeding to gateway startup.\n";
        return true;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "[BootPhase] Boot failed: " << ex.what() << std::endl;
        return false;
    }
}
