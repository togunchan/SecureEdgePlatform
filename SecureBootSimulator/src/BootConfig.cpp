#include "secureboot/BootConfig.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <limits>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>

namespace secureboot
{
    namespace
    {
        std::string trimCopy(const std::string &value)
        {
            const auto begin = value.find_first_not_of(" \t\n\r");
            if (begin == std::string::npos)
            {
                return {};
            }

            const auto end = value.find_last_not_of(" \t\n\r");
            return value.substr(begin, end - begin + 1);
        }

        bool isHexString(const std::string &value)
        {
            return !value.empty() && std::all_of(value.begin(), value.end(), [](unsigned char ch)
                                                  { return std::isxdigit(ch) != 0; });
        }

        bool isDecimalString(const std::string &value)
        {
            return !value.empty() && std::all_of(value.begin(), value.end(), [](unsigned char ch)
                                                  { return std::isdigit(ch) != 0; });
        }

        uint32_t parseEntryPoint(const std::string &raw)
        {
            const std::string clean = trimCopy(raw);
            if (clean.empty())
            {
                throw std::runtime_error("Entry point must not be empty.");
            }

            auto makeError = [&clean](const std::string &reason)
            {
                return std::runtime_error("Invalid entry_point format '" + clean + "': " + reason);
            };

            try
            {
                unsigned long long value = 0;
                if (clean.rfind("0x", 0) == 0 || clean.rfind("0X", 0) == 0)
                {
                    const std::string hexPart = clean.substr(2);
                    if (!isHexString(hexPart))
                    {
                        throw makeError("expected hexadecimal digits after 0x prefix");
                    }

                    value = std::stoull(hexPart, nullptr, 16);
                }
                else
                {
                    if (!isDecimalString(clean))
                    {
                        throw makeError("expected decimal digits");
                    }
                    value = std::stoull(clean, nullptr, 10);
                }

                if (value > std::numeric_limits<uint32_t>::max())
                {
                    throw makeError("value out of range for 32-bit entry point");
                }

                return static_cast<uint32_t>(value);
            }
            catch (const std::runtime_error &)
            {
                throw;
            }
            catch (const std::exception &ex)
            {
                throw makeError(ex.what());
            }
        }
    } // namespace

    bool BootConfig::loadFromFile(const std::string &path)
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

        if (!json["firmware_path"].is_string() || !json["expected_sha256"].is_string() ||
            !json["boot_mode"].is_string())
        {
            throw std::runtime_error("Config fields must be strings: firmware_path, expected_sha256, boot_mode");
        }

        firmwarePath_ = trimCopy(json["firmware_path"].get<std::string>());
        expectedSha256_ = trimCopy(json["expected_sha256"].get<std::string>());
        bootMode_ = trimCopy(json["boot_mode"].get<std::string>());

        if (firmwarePath_.empty())
        {
            throw std::runtime_error("Config field firmware_path must not be empty");
        }
        if (expectedSha256_.empty())
        {
            throw std::runtime_error("Config field expected_sha256 must not be empty");
        }
        if (expectedSha256_.size() != 64 || !isHexString(expectedSha256_))
        {
            throw std::runtime_error("Config field expected_sha256 must be a 64-character hexadecimal string");
        }
        if (bootMode_.empty())
        {
            throw std::runtime_error("Config field boot_mode must not be empty");
        }

        if (json["entry_point"].is_string())
        {
            entryPointStr_ = trimCopy(json["entry_point"].get<std::string>());
        }
        else if (json["entry_point"].is_number_unsigned())
        {
            entryPointStr_ = std::to_string(json["entry_point"].get<uint64_t>());
        }
        else if (json["entry_point"].is_number_integer())
        {
            const int64_t value = json["entry_point"].get<int64_t>();
            if (value < 0)
            {
                throw std::runtime_error("Config field entry_point must be non-negative");
            }
            entryPointStr_ = std::to_string(static_cast<uint64_t>(value));
        }
        else
        {
            throw std::runtime_error("Config field entry_point must be a string or integer");
        }

        // Resolve firmware path relative to the config file location when necessary
        std::filesystem::path resolvedPath = firmwarePath_;
        if (resolvedPath.is_relative())
        {
            const std::filesystem::path configDir = std::filesystem::path(path).parent_path();
            resolvedPath = std::filesystem::weakly_canonical(configDir / resolvedPath);
        }
        firmwarePath_ = resolvedPath.lexically_normal().string();

        if (!std::filesystem::exists(firmwarePath_))
        {
            throw std::runtime_error("Firmware image not found at: " + firmwarePath_);
        }

        // Validate entry point eagerly to surface configuration errors early.
        (void)parseEntryPoint(entryPointStr_);

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

    uint32_t BootConfig::getEntryPoint() const
    {
        return parseEntryPoint(entryPointStr_);
    }

} // namespace secureboot
