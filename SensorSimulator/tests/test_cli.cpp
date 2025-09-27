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

TEST_CASE("CLI list shows all added sensors", "[cli][list]")
{
    EdgeShell shell;

    // Add two sensors
    shell.addScheduledSensor("TEMP-010", 500);
    shell.addScheduledSensor("PRES-020", 1000);

    // Create a stringstream to capture output.
    // Instead of printing to the console, we'll redirect output into this buffer.
    std::stringstream buffer;

    // Save the current std::cout buffer (which normally writes to the console).
    // This allows us to restore it later after capturing the output.
    std::streambuf *oldCout = std::cout.rdbuf(buffer.rdbuf());

    // Redirect std::cout to write into our stringstream buffer.
    // Any output from listSensors() will now go into 'buffer' instead of the console.
    shell.listSensors();

    // Restore std::cout to its original state (back to console output).
    std::cout.rdbuf(oldCout);

    // Extract the captured output from the buffer as a string.
    // We'll use this string to verify that the expected sensor names appear.
    std::string output = buffer.str();

    REQUIRE(output.find("TEMP-010") != std::string::npos);
    REQUIRE(output.find("PRES-020") != std::string::npos);
}

TEST_CASE("CLI reset restores determinism", "[cli][reset]")
{
    EdgeShell shell;
    shell.addScheduledSensor("TEMP-001", 1000);

    auto &sensors = shell.getSensors();
    auto *sensor = sensors.at("TEMP-001").get();

    sensor->reset(42);
    auto s1 = sensor->nextSample(1000);

    sensor->reset(42);
    auto s2 = sensor->nextSample(1000);
    REQUIRE(s1.value == Catch::Approx(s2.value).epsilon(1e-12));
}

TEST_CASE("CLI step command outputs correct sensor", "[cli][step]")
{
    EdgeShell shell;
    shell.addScheduledSensor("TEMP-001", 1000);

    std::stringstream buffer;
    std::streambuf *old = std::cout.rdbuf(buffer.rdbuf());

    shell.stepSensor("TEMP-001");

    std::cout.rdbuf(old);
    std::string output = buffer.str();

    REQUIRE(output.find("TEMP-001") != std::string::npos);
}

TEST_CASE("CLI step all outputs all sensors", "[cli][step][all]")
{
    EdgeShell shell;
    shell.addScheduledSensor("TEMP-001", 1000);
    shell.addScheduledSensor("PRES-001", 1000);

    std::stringstream buffer;
    std::streambuf *old = std::cout.rdbuf(buffer.rdbuf());

    shell.stepAllSensors();

    std::cout.rdbuf(old);
    std::string output = buffer.str();

    REQUIRE(output.find("TEMP-001") != std::string::npos);
    REQUIRE(output.find("PRES-001") != std::string::npos);
}

TEST_CASE("CLI tick advances time and samples", "[cli][tick]")
{
    EdgeShell shell;
    shell.addScheduledSensor("TEMP-001", 1000);

    std::stringstream buffer;
    std::streambuf *old = std::cout.rdbuf(buffer.rdbuf());

    shell.tickTime(1000);

    std::cout.rdbuf(old);
    std::string output = buffer.str();

    REQUIRE(output.find("TEMP-001") != std::string::npos);
    REQUIRE(output.find("[Tick @ 1000]") != std::string::npos);
}

TEST_CASE("CLI tick advances simulation time", "[cli][tick]")
{
    EdgeShell shell;
    shell.addScheduledSensor("TEMP-001", 500);

    std::stringstream buffer;
    std::streambuf *oldCout = std::cout.rdbuf(buffer.rdbuf());

    shell.tickTime(1000);

    std::cout.rdbuf(oldCout);

    std::string output = buffer.str();
    REQUIRE(output.find("TEMP-001") != std::string::npos);
    REQUIRE(output.find("value:") != std::string::npos);
    REQUIRE(output.find("1000 ms") != std::string::npos);
}

TEST_CASE("CLI run and stop real-time simulation via REPL", "[cli][run][stop][repl]")
{
    EdgeShell shell;
    shell.addScheduledSensor("TEMP-001", 500);

    std::istringstream fakeInput("run\nstop\nexit\n");
    std::streambuf *oldCin = std::cin.rdbuf(fakeInput.rdbuf());

    std::stringstream buffer;
    std::streambuf *oldCout = std::cout.rdbuf(buffer.rdbuf());

    shell.run();

    std::cin.rdbuf(oldCin);
    std::cout.rdbuf(oldCout);

    std::string output = buffer.str();
    REQUIRE(output.find("Started real-time simulation") != std::string::npos);
    REQUIRE(output.find("Stopped real-time simulation.") != std::string::npos);
    REQUIRE(output.find("TEMP-001") != std::string::npos);
}