#include <catch2/catch_all.hpp>
#include <secureboot/BootSimulator.hpp>
#include <secureboot/BootConfig.hpp>
#include <secureboot/BootStage.hpp>

using namespace secureboot;

TEST_CASE("Successful boot flow with valid config and firmware hash")
{

    std::string fullPath = std::string(DATA_DIR_PATH) + "/good_config.json";

    BootConfig config;
    config.loadFromFile(fullPath);

    BootSimulator simulator(config, HashMethod::SHA256);

    BootStage stage1("Initialize Hardwire", 1, []() {});

    BootStage stage2("Load Drivers", 1, []() {});

    simulator.addStage(stage1);
    simulator.addStage(stage2);

    simulator.run();

    CHECK(simulator.wasSuccessful() == true);
    CHECK(simulator.getFailureReason().empty());
}