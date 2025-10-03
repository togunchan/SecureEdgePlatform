#include "secureboot/BootConfig.hpp"
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <sstream>

namespace secureboot
{
    bool BootConfig::loadFromFile(std::string &path)
    {
        std::ifstream file(path);
        if (!file)
        {
            throw std::runtime_error("Failed to open config file: " + path);
        }

        nlohmann::json json;
        try
        {
            file >> json;
        }
        catch (const std::exception &e)
        {
            throw std::runtime_error("Invalid JSON format: " + std::string(e.what()));
        }

        if (!json.contains("firmware_path") || !json.contains("expected_sha256") ||
            !json.contains("boot_mode") || !json.contains("entry_point"))
        {
            throw std::runtime_error("Missing required field(s) in JSON config");
        }

        firmwarePath_ = json["firmware_path"].get<std::string>();
        expectedSha256_ = json["expected_sha256"].get<std::string>();
        bootMode_ = json["boot_mode"].get<std::string>();
        entryPointStr_ = json["entry_point"].get<std::string>();

        return true;
    }

    const std::string &BootConfig::getFirmwarePath() const
    {
        return firmwarePath_;
    }

    const std::string &BootConfig::getExpectedSha256() const
    {
        return expectedSha256_;
    }

    const std::string &BootConfig::getBootMode() const
    {
        return bootMode_;
    }

    uint32_t &BootConfig::getEntryPoint() const
    {
        uint32_t entry = 0;

        try
        {
            std::string cleanStr = entryPointStr_;
            if (cleanStr.starts_with("0x" || cleanStr.starts_with("0X")))
            {
                cleanStr = cleanStr.substr(2);
            }

            std::stringstream ss;
            ss << std::hex << cleanStr;
            ss >> entry;
        }
        catch (const std::exception &e)
        {
            throw std::runtime_error("Invalid entry_point format: " + entryPointStr_);
        }
    }

} // namespace secureboot
