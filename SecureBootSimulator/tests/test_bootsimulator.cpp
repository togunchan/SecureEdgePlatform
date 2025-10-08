#include <catch2/catch_all.hpp>
#include <secureboot/BootConfig.hpp>
#include <secureboot/BootSimulator.hpp>
#include <secureboot/BootStage.hpp>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>

#include "TestPaths.hpp"

using namespace secureboot;

namespace
{
    std::filesystem::path makeUniqueConfigPath(const std::filesystem::path &directory)
    {
        static std::atomic<uint64_t> counter{0};
        const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        return directory / ("sim_config_" + std::to_string(timestamp) + "_" + std::to_string(counter++) + ".json");
    }

    class TempJsonFile
    {
    public:
        TempJsonFile(const std::filesystem::path &directory, const std::string &content)
        {
            std::filesystem::create_directories(directory);
            path_ = makeUniqueConfigPath(directory);
            std::ofstream output(path_);
            output << content;
        }

        ~TempJsonFile()
        {
            std::error_code ec;
            std::filesystem::remove(path_, ec);
        }

        const std::filesystem::path &path() const
        {
            return path_;
        }

    private:
        std::filesystem::path path_;
    };

    std::string buildConfig(const std::string &firmwarePath,
                            const std::string &expectedSha,
                            const std::string &entryPoint = "0x0")
    {
        return "{\n"
               "  \"firmware_path\": \"" + firmwarePath + "\",\n"
               "  \"expected_sha256\": \"" + expectedSha + "\",\n"
               "  \"boot_mode\": \"NORMAL\",\n"
               "  \"entry_point\": \"" + entryPoint + "\"\n"
               "}\n";
    }
} // namespace

TEST_CASE("BootSimulator runs stages in order and reports success")
{
    const std::filesystem::path configPath = test_support::dataDirectory() / "good_config.json";

    BootConfig config;
    REQUIRE(config.loadFromFile(configPath.string()));

    BootSimulator simulator(config, HashMethod::SHA256);

    std::vector<std::string> executed;

    BootStage stageB("StageB", 2, [&]() { executed.emplace_back("B"); });
    BootStage stageA("StageA", 1, [&]() { executed.emplace_back("A"); });

    simulator.addStage(stageB);
    simulator.addStage(stageA);

    simulator.run();

    REQUIRE(simulator.wasSuccessful());
    REQUIRE(simulator.getFailureReason().empty());
    REQUIRE(executed == std::vector<std::string>{"A", "B"});
}

TEST_CASE("BootSimulator stops on stage failure and surfaces error code")
{
    const std::filesystem::path configPath = test_support::dataDirectory() / "good_config.json";

    BootConfig config;
    REQUIRE(config.loadFromFile(configPath.string()));

    BootSimulator simulator(config);

    std::atomic<bool> failCallbackInvoked{false};
    BootStage failingStage("Failing", 1,
                           [&]()
                           { throw StageFailure(11, "stage failure"); },
                           [&]()
                           { failCallbackInvoked = true; });
    BootStage skippedStage("Skipped", 2, []()
                           { FAIL("Stage executed despite previous failure"); });

    simulator.addStage(failingStage);
    simulator.addStage(skippedStage);

    simulator.run();

    REQUIRE_FALSE(simulator.wasSuccessful());
    REQUIRE(simulator.getFailureReason().find("11") != std::string::npos);
    REQUIRE(failCallbackInvoked.load());
}

TEST_CASE("BootSimulator handles missing stage handlers as failure")
{
    const std::filesystem::path configPath = test_support::dataDirectory() / "good_config.json";

    BootConfig config;
    REQUIRE(config.loadFromFile(configPath.string()));

    BootSimulator simulator(config);
    BootStage missingHandler("MissingHandler", 1);

    simulator.addStage(missingHandler);

    simulator.run();

    REQUIRE_FALSE(simulator.wasSuccessful());
    REQUIRE(simulator.getFailureReason().find("MissingHandler") != std::string::npos);
    REQUIRE(simulator.getFailureReason().find(std::to_string(BootStage::kMissingHandlerErrorCode)) != std::string::npos);
}

TEST_CASE("BootSimulator catches firmware hash mismatches")
{
    const auto firmwarePath = (test_support::dataDirectory() / "fake_firmware.bin").string();
    const TempJsonFile configFile(std::filesystem::temp_directory_path(),
                                  buildConfig(firmwarePath, std::string(64, 'a')));

    BootConfig config;
    REQUIRE(config.loadFromFile(configFile.path().string()));

    BootSimulator simulator(config);

    simulator.run();

    REQUIRE_FALSE(simulator.wasSuccessful());
    REQUIRE(simulator.getFailureReason() == "Firmware signature mismatch.");
}

TEST_CASE("BootSimulator surfaces firmware verification exceptions")
{
    const std::filesystem::path configPath = test_support::dataDirectory() / "good_config.json";

    BootConfig config;
    REQUIRE(config.loadFromFile(configPath.string()));

    BootSimulator simulator(config, HashMethod::CRC32);

    simulator.run();

    REQUIRE_FALSE(simulator.wasSuccessful());
    REQUIRE(simulator.getFailureReason().find("Firmware verification error") != std::string::npos);
}

TEST_CASE("BootSimulator reports missing stages")
{
    const std::filesystem::path configPath = test_support::dataDirectory() / "good_config.json";

    BootConfig config;
    REQUIRE(config.loadFromFile(configPath.string()));

    BootSimulator simulator(config);

    simulator.run();

    REQUIRE_FALSE(simulator.wasSuccessful());
    REQUIRE(simulator.getFailureReason() == "No boot stages configured.");
}

TEST_CASE("BootSimulator clears failure state on subsequent success")
{
    const std::filesystem::path configPath = test_support::dataDirectory() / "good_config.json";

    BootConfig config;
    REQUIRE(config.loadFromFile(configPath.string()));

    BootSimulator simulator(config);

    std::atomic<bool> shouldFail{true};
    BootStage stage("Flaky", 1, [&]()
                    {
                        if (shouldFail.exchange(false))
                        {
                            throw StageFailure(55, "transient");
                        }
                    });

    simulator.addStage(stage);

    simulator.run();
    REQUIRE_FALSE(simulator.wasSuccessful());
    REQUIRE(simulator.getFailureReason().find("55") != std::string::npos);

    simulator.run();
    REQUIRE(simulator.wasSuccessful());
    REQUIRE(simulator.getFailureReason().empty());
}
