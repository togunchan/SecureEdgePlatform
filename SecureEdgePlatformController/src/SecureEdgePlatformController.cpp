#include <SecureEdgePlatformController.hpp>
#include <iostream>

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
