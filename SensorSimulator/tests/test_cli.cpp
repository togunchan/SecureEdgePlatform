#include <catch2/catch_all.hpp>
#include "cli/EdgeShell.hpp"
#include <sstream>

using namespace sensor;

TEST_CASE("CLI add TEMP sensor", "[cli][add]")
{
    EdgeShell shell;

    shell.addScheduledSensor("TEMP-001", 500);

    auto &sensors = shell.getSensors();
    REQUIRE(sensors.find("TEMP-001") != sensors.end());
    REQUIRE(sensors.at("TEMP-001")->type() == "TEMP");
    REQUIRE(sensors.at("TEMP-001")->rateHz() == 10);
}

TEST_CASE("CLI add PRES sensor", "[cli][add]")
{
    EdgeShell shell;

    shell.addScheduledSensor("PRES-001", 1000);

    auto &sensors = shell.getSensors();
    REQUIRE(sensors.find("PRES-001") != sensors.end());
    REQUIRE(sensors.at("PRES-001")->type() == "PRES");
    REQUIRE(sensors.at("PRES-001")->rateHz() == 1);
}

TEST_CASE("CLI rejects unknown sensor type", "[cli][validation]")
{
    EdgeShell shell;

    std::stringstream buffer;
    std::streambuf *oldCout = std::cout.rdbuf(buffer.rdbuf());

    shell.addScheduledSensor("HUM-001", 1000);

    std::cout.rdbuf(oldCout);

    std::string output = buffer.str();
    REQUIRE(output.find("Only TEMP and PRES") != std::string::npos);
}

TEST_CASE("CLI case-insensitive sensor IDs", "[cli][add][case]")
{
    EdgeShell shell;

    shell.addScheduledSensor("temp-002", 750);
    shell.addScheduledSensor("PrEs-003", 2000);

    auto &sensors = shell.getSensors();
    REQUIRE(sensors.find("temp-002") != sensors.end());
    REQUIRE(sensors.find("PrEs-003") != sensors.end());

    REQUIRE(sensors.at("temp-002")->type() == "TEMP");
    REQUIRE(sensors.at("PrEs-003")->type() == "PRES");
}

TEST_CASE("CLI remove sensor", "[cli][remove]")
{
    EdgeShell shell;
    shell.addScheduledSensor("TEMP-001", 1000);

    auto &sensors = shell.getSensors();
    REQUIRE(sensors.find("TEMP-001") != sensors.end());

    bool removed = shell.removeSensor("TEMP-001");
    REQUIRE(removed);

    auto &sensors2 = shell.getSensors();
    REQUIRE(sensors2.find("TEMP-001") == sensors2.end());
}