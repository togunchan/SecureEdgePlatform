#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <AgentChannel.hpp>
#include <EdgeGateway.hpp>
#include <edgeagent/EdgeAgent.hpp>
#include <cppminidb/SensorLogRow.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <future>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
namespace
{
    std::filesystem::path makeTempDir(const std::string &prefix)
    {
        const auto base = std::filesystem::temp_directory_path();
        const auto timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        for (int attempt = 0; attempt < 8; ++attempt)
        {
            auto candidate = base / (prefix + "-" + std::to_string(timestamp) + "-" + std::to_string(attempt));
            std::error_code ec;
            if (std::filesystem::create_directories(candidate, ec))
            {
                return candidate;
            }
            if (!std::filesystem::exists(candidate))
            {
                continue;
            }
            std::filesystem::remove_all(candidate, ec);
            if (std::filesystem::create_directories(candidate, ec))
            {
                return candidate;
            }
        }
        throw std::runtime_error("Failed to create temporary directory for integration test");
    }
    void removeAll(const std::filesystem::path &path)
    {
        std::error_code ec;
        std::filesystem::remove_all(path, ec);
    }
    void writeJson(const std::filesystem::path &path, const nlohmann::json &payload)
    {
        std::error_code ec;
        std::filesystem::create_directories(path.parent_path(), ec);
        std::ofstream out(path);
        if (!out)
        {
            throw std::runtime_error("Failed to open file for writing: " + path.string());
        }
        out << payload.dump(4);
    }
    std::string readFile(const std::filesystem::path &path)
    {
        std::ifstream in(path);
        if (!in)
        {
            throw std::runtime_error("Failed to open file for reading: " + path.string());
        }
        std::ostringstream buffer;
        buffer << in.rdbuf();
        return buffer.str();
    }
    std::size_t countOccurrences(const std::string &text, const std::string &needle)
    {
        if (needle.empty())
        {
            return 0;
        }
        std::size_t count = 0;
        std::size_t pos = text.find(needle);
        while (pos != std::string::npos)
        {
            ++count;
            pos = text.find(needle, pos + needle.size());
        }
        return count;
    }
} // namespace
TEST_CASE("EdgeGateway publishes samples to configured file and agent channels", "[integration][gateway]")
{
    const auto tempDir = makeTempDir("secure-edge-platform");
    const auto configPath = tempDir / "gateway_config.json";
    const auto fileChannelPath = tempDir / "telemetry.ndjson";
    const auto agentDumpPath = tempDir / "agent_dump.json";
    const nlohmann::json config = {
        {"channels",
         nlohmann::json::array({
             nlohmann::json{{"type", "console"}},
             nlohmann::json{{"type", "file"}, {"path", "telemetry.ndjson"}},
             nlohmann::json{{"type", "agent"}},
         })}};
    writeJson(configPath, config);
    gateway::EdgeGateway gateway;
    gateway.start(configPath.string());
    edgeagent::EdgeAgent probeAgent;
    gateway.setChannelsForTest(std::make_unique<channel::AgentChannel>(&probeAgent));
    gateway.setSampleCallbackForTest();
    cppminidb::SensorLogRow first{1'000, "TEMP-001", 42.5, {"stuck"}};
    cppminidb::SensorLogRow second{2'000, "TEMP-001", 41.25, {}};
    gateway.injectTestSample(first);
    gateway.injectTestSample(second);
    REQUIRE(std::filesystem::exists(fileChannelPath));
    const auto content = readFile(fileChannelPath);
    CHECK(countOccurrences(content, "\"sensorId\": \"TEMP-001\"") == 2);
    CHECK(content.find("\"value\": 42.5") != std::string::npos);
    probeAgent.flushToFile(agentDumpPath.string());
    REQUIRE(std::filesystem::exists(agentDumpPath));
    std::ifstream agentStream(agentDumpPath);
    REQUIRE(agentStream.good());
    nlohmann::json agentJson;
    agentStream >> agentJson;
    REQUIRE(agentJson.is_array());
    REQUIRE(agentJson.size() == 2);
    CHECK(agentJson[0]["sensor_id"] == "TEMP-001");
    CHECK(agentJson[0]["value"] == Catch::Approx(42.5));
    CHECK(agentJson[1]["sensor_id"] == "TEMP-001");
   CHECK(agentJson[1]["value"] == Catch::Approx(41.25));
   removeAll(tempDir);
}
TEST_CASE("EdgeGateway ignores invalid channel entries but still publishes via valid ones", "[integration][gateway][config]")
{
    const auto tempDir = makeTempDir("secure-edge-platform-invalid-channels");
    const auto configPath = tempDir / "gateway_config.json";
    const auto validFilePath = tempDir / "valid-output.ndjson";
    const nlohmann::json config = {
        {"channels",
         nlohmann::json::array({
             nlohmann::json{{"type", "unknown"}},
             nlohmann::json{{"type", "file"}},
             nlohmann::json{{"type", "file"}, {"path", validFilePath.string()}},
         })}};
    writeJson(configPath, config);
    gateway::EdgeGateway gateway;
    gateway.start(configPath.string());
    gateway.setSampleCallbackForTest();
    cppminidb::SensorLogRow sample{3'000, "PRESS-007", 12.75, {"calibrating"}};
    gateway.injectTestSample(sample);
    REQUIRE(std::filesystem::exists(validFilePath));
    const auto content = readFile(validFilePath);
    CHECK(countOccurrences(content, "\"sensorId\": \"PRESS-007\"") == 1);
    CHECK(content.find("\"value\": 12.75") != std::string::npos);
    removeAll(tempDir);
}
TEST_CASE("EdgeGateway runLoop produces telemetry using scheduled sensors", "[integration][gateway][scheduler]")
{
    const auto tempDir = makeTempDir("secure-edge-platform-loop");
    const auto configPath = tempDir / "gateway_config.json";
    const auto filePath = tempDir / "scheduled-output.ndjson";
    const nlohmann::json config = {
        {"channels",
         nlohmann::json::array({
             nlohmann::json{{"type", "file"}, {"path", "scheduled-output.ndjson"}},
         })}};
    writeJson(configPath, config);
    gateway::EdgeGateway gateway;
    gateway.start(configPath.string());
    std::thread loopThread([&gateway]()
                           { gateway.runLoop(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    gateway.stopLoop();
    loopThread.join();
    REQUIRE(std::filesystem::exists(filePath));
    const auto content = readFile(filePath);
    REQUIRE_FALSE(content.empty());
    CHECK(content.find("\"sensorId\": \"TEMP-001\"") != std::string::npos);
    removeAll(tempDir);
}
TEST_CASE("EdgeGateway runLoop prevents concurrent restarts", "[integration][gateway][concurrency]")
{
    const auto tempDir = makeTempDir("secure-edge-platform-runloop");
    const auto configPath = tempDir / "gateway_config.json";
    const auto filePath = tempDir / "runloop-output.ndjson";
    const nlohmann::json config = {
        {"channels",
         nlohmann::json::array({
             nlohmann::json{{"type", "file"}, {"path", filePath.string()}},
         })}};
    writeJson(configPath, config);
    gateway::EdgeGateway gateway;
    gateway.start(configPath.string());

    std::thread loopThread([&gateway]()
                           { gateway.runLoop(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    auto duplicateRun = std::async(std::launch::async, [&gateway]()
                                   { gateway.runLoop(); });
    auto status = duplicateRun.wait_for(std::chrono::milliseconds(250));
    REQUIRE(status == std::future_status::ready);
    REQUIRE_NOTHROW(duplicateRun.get());

    gateway.stopLoop();
    loopThread.join();

    REQUIRE(std::filesystem::exists(filePath));
    const auto content = readFile(filePath);
    REQUIRE_FALSE(content.empty());
    removeAll(tempDir);
}
TEST_CASE("EdgeGateway start is idempotent for default sensor scheduling", "[integration][gateway][scheduler]")
{
    const auto tempDir = makeTempDir("secure-edge-platform-idempotent-start");
    const auto configPath = tempDir / "gateway_config.json";
    const auto filePath = tempDir / "idempotent-output.ndjson";
    const nlohmann::json config = {
        {"channels",
         nlohmann::json::array({
             nlohmann::json{{"type", "file"}, {"path", filePath.string()}},
         })}};
    writeJson(configPath, config);
    gateway::EdgeGateway gateway;
    gateway.start(configPath.string());
    const auto firstIds = gateway.getScheduler().getSensorIds();
    REQUIRE(firstIds.size() == 1);
    CHECK(firstIds.front() == "TEMP-001");

    gateway.start(configPath.string());
    const auto secondIds = gateway.getScheduler().getSensorIds();
    REQUIRE(secondIds.size() == 1);
    CHECK(secondIds.front() == "TEMP-001");

    gateway.setSampleCallbackForTest();
    cppminidb::SensorLogRow sample{4'000, "TEMP-001", 55.5, {}};
    gateway.injectTestSample(sample);
    REQUIRE(std::filesystem::exists(filePath));
    const auto content = readFile(filePath);
    CHECK(countOccurrences(content, "\"sensorId\": \"TEMP-001\"") == 1);
    CHECK(content.find("\"value\": 55.5") != std::string::npos);
    removeAll(tempDir);
}
