#include <edgeagent/EdgeAgent.hpp>
#include <cppminidb/SensorLogRow.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <system_error>
#include <chrono>

namespace
{
    std::filesystem::path makeTempFilePath()
    {
        auto tempDir = std::filesystem::temp_directory_path();
        static std::size_t counter = 0;
        auto timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        auto uniqueName = "edgeagent-" + std::to_string(timestamp) + "-" + std::to_string(counter++) + ".json";
        auto path = tempDir / uniqueName;

        std::error_code ec;
        std::filesystem::remove(path, ec);
        return path;
    }

    void removeIfExists(const std::filesystem::path &path)
    {
        std::error_code ec;
        std::filesystem::remove(path, ec);
    }
} // namespace

TEST_CASE("flushToConsole clears buffered data", "[edgeagent]")
{
    edgeagent::EdgeAgent agent;

    agent.receive({1000, "sensor-A", 42.0, {"spike"}});
    agent.receive({2000, "sensor-B", 36.5, {}});

    REQUIRE_NOTHROW(agent.flushToConsole());

    auto outputPath = makeTempFilePath();
    agent.flushToFile(outputPath.string());

    REQUIRE_FALSE(std::filesystem::exists(outputPath));
}

TEST_CASE("flushToFile writes buffered rows as JSON and empties buffer", "[edgeagent]")
{
    edgeagent::EdgeAgent agent;

    agent.receive({1234, "sensor-X", 27.3, {"stuck", "spike"}});
    agent.receive({1240, "sensor-Y", 31.4, {}});

    auto outputPath = makeTempFilePath();
    REQUIRE_NOTHROW(agent.flushToFile(outputPath.string()));
    REQUIRE(std::filesystem::exists(outputPath));

    std::ifstream file(outputPath);
    REQUIRE(file.is_open());

    nlohmann::json data;
    file >> data;

    REQUIRE(data.is_array());
    REQUIRE(data.size() == 2);
    CHECK(data[0]["sensor_id"] == "sensor-X");
    CHECK(data[0]["fault_flags"].is_array());
    CHECK(data[0]["fault_flags"].size() == 2);
    CHECK(data[1]["sensor_id"] == "sensor-Y");

    removeIfExists(outputPath);

    agent.flushToFile(outputPath.string());
    REQUIRE_FALSE(std::filesystem::exists(outputPath));
}

TEST_CASE("flushToFile preserves buffered data when write fails", "[edgeagent]")
{
    edgeagent::EdgeAgent agent;
    agent.receive({5678, "sensor-Z", 99.9, {}});

    auto invalidPath = std::filesystem::temp_directory_path() / "edgeagent-nonexistent" / "telemetry.json";
    REQUIRE_FALSE(std::filesystem::exists(invalidPath.parent_path()));
    REQUIRE_NOTHROW(agent.flushToFile(invalidPath.string()));

    auto recoveryPath = makeTempFilePath();
    REQUIRE_NOTHROW(agent.flushToFile(recoveryPath.string()));
    REQUIRE(std::filesystem::exists(recoveryPath));

    std::ifstream file(recoveryPath);
    REQUIRE(file.is_open());

    nlohmann::json data;
    file >> data;

    REQUIRE(data.is_array());
    REQUIRE(data.size() == 1);
    CHECK(data[0]["sensor_id"] == "sensor-Z");
    CHECK(data[0]["value"] == Catch::Approx(99.9));

    removeIfExists(recoveryPath);
}
