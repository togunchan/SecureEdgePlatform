#include <catch2/catch_all.hpp>
#include <secureboot/BootConfig.hpp>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>

#include "TestPaths.hpp"

using namespace secureboot;

namespace
{
    constexpr const char *kEmptySha256 = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

    std::filesystem::path makeUniqueJsonPath(const std::filesystem::path &directory, const std::string &prefix)
    {
        static std::atomic<uint64_t> counter{0};
        const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        return directory / (prefix + std::to_string(timestamp) + "_" + std::to_string(counter++) + ".json");
    }

    class TempJsonFile
    {
    public:
        TempJsonFile(const std::filesystem::path &directory, const std::string &content)
        {
            std::filesystem::create_directories(directory);
            path_ = makeUniqueJsonPath(directory, "config_");
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

    std::string buildConfigJson(const std::string &firmwarePath,
                                const std::string &expectedSha,
                                const std::string &entryPoint,
                                const std::string &bootMode = "NORMAL")
    {
        std::ostringstream oss;
        oss << "{\n"
            << "  \"firmware_path\": \"" << firmwarePath << "\",\n"
            << "  \"expected_sha256\": \"" << expectedSha << "\",\n"
            << "  \"boot_mode\": \"" << bootMode << "\",\n"
            << "  \"entry_point\": \"" << entryPoint << "\"\n"
            << "}\n";
        return oss.str();
    }
} // namespace

TEST_CASE("BootConfig loads valid configuration with absolute firmware path")
{
    const auto dataDir = test_support::dataDirectory();
    const auto firmwarePath = (dataDir / "fake_firmware.bin").lexically_normal();
    const TempJsonFile configFile(std::filesystem::temp_directory_path(),
                                  buildConfigJson(firmwarePath.string(), kEmptySha256, "0x1000"));

    BootConfig config;
    REQUIRE(config.loadFromFile(configFile.path().string()));

    CHECK(config.getFirmwarePath() == firmwarePath.string());
    CHECK(config.getExpectedSha256() == kEmptySha256);
    CHECK(config.getBootMode() == "NORMAL");
    CHECK(config.getEntryPoint() == 0x1000);
}

TEST_CASE("BootConfig resolves firmware path relative to config directory")
{
    const auto dataDir = test_support::dataDirectory();
    const auto firmwarePath = (dataDir / "fake_firmware.bin").lexically_normal();
    const TempJsonFile configFile(dataDir,
                                  buildConfigJson("fake_firmware.bin", kEmptySha256, "4096"));

    BootConfig config;
    REQUIRE(config.loadFromFile(configFile.path().string()));

    CHECK(config.getFirmwarePath() == firmwarePath.string());
    CHECK(config.getEntryPoint() == 4096);
}

TEST_CASE("BootConfig rejects invalid expected SHA256 strings")
{
    const auto dataDir = test_support::dataDirectory();
    const TempJsonFile configFile(std::filesystem::temp_directory_path(),
                                  buildConfigJson((dataDir / "fake_firmware.bin").string(),
                                                  "ABC", "0x0"));

    BootConfig config;
    REQUIRE_THROWS_AS(config.loadFromFile(configFile.path().string()), std::runtime_error);
}

TEST_CASE("BootConfig rejects missing fields in configuration")
{
    const auto dataDir = test_support::dataDirectory();
    const std::string json = "{ \"firmware_path\": \"" + (dataDir / "fake_firmware.bin").string() + "\" }";
    const TempJsonFile configFile(std::filesystem::temp_directory_path(), json);

    BootConfig config;
    REQUIRE_THROWS_AS(config.loadFromFile(configFile.path().string()), std::runtime_error);
}

TEST_CASE("BootConfig rejects invalid entry point strings")
{
    const auto dataDir = test_support::dataDirectory();
    const TempJsonFile configFile(std::filesystem::temp_directory_path(),
                                  buildConfigJson((dataDir / "fake_firmware.bin").string(),
                                                  kEmptySha256, "main"));

    BootConfig config;
    REQUIRE_THROWS_AS(config.loadFromFile(configFile.path().string()), std::runtime_error);
}

TEST_CASE("BootConfig rejects negative entry points")
{
    const auto dataDir = test_support::dataDirectory();
    std::ostringstream json;
    json << "{\n"
         << "  \"firmware_path\": \"" << (dataDir / "fake_firmware.bin").string() << "\",\n"
         << "  \"expected_sha256\": \"" << kEmptySha256 << "\",\n"
         << "  \"boot_mode\": \"NORMAL\",\n"
         << "  \"entry_point\": -1\n"
         << "}\n";
    const TempJsonFile configFile(std::filesystem::temp_directory_path(), json.str());

    BootConfig config;
    REQUIRE_THROWS_AS(config.loadFromFile(configFile.path().string()), std::runtime_error);
}
