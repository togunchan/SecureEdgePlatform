#include <GatewayConfig.hpp>
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
#include <filesystem>

namespace channel
{
    bool GatewayConfig::loadFromFile(const std::string &path)
    {
        std::ifstream file(path);
        if (!file)
        {
            std::cerr << "[GatewayConfig] Failed to open file: " << path << "\n";
            return false;
        };

        nlohmann::json j;
        file >> j;

        if (!j.contains("channels") || !j["channels"].is_array())
        {
            std::cerr << "[GatewayConfig] Invalid config: 'channels' array missing.\n";
            return false;
        }

        const std::filesystem::path baseDir = std::filesystem::path(path).parent_path();

        channels_.clear();
        for (const auto &chnl : j["channels"])
        {
            ChannelConfig cfg;
            cfg.type = chnl.value("type", "");
            cfg.path = chnl.value("path", "");
            if (!cfg.path.empty())
            {
                std::filesystem::path resolved(cfg.path);
                if (resolved.is_relative())
                {
                    resolved = baseDir / resolved;
                }
                cfg.path = resolved.lexically_normal().string();
            }
            channels_.push_back(cfg);
        }

        return true;
    }

    const std::vector<ChannelConfig> &GatewayConfig::getChannels() const
    {
        return channels_;
    }

} // namespace channel
