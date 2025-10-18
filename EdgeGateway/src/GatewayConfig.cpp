#include <GatewayConfig.hpp>
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>

namespace channel
{
    bool GatewayConfig::loadFromFile(const std::string &filePath)
    {
        std::ifstream file(filePath);
        if (!file)
        {
            std::cerr << "[GatewayConfig] Failed to open file: " << filePath << "\n";
            return false;
        };

        nlohmann::json j;
        file >> j;

        if (!j.contains("channels") || !j["channels"].is_array())
        {
            std::cerr << "[GatewayConfig] Invalid config: 'channels' array missing.\n";
            return false;
        }

        channels_.clear();
        for (const auto &chnl : j["channels"])
        {
            ChannelConfig cfg;
            cfg.type = chnl.value("type", "");
            cfg.path = chnl.value("path", "");
            channels_.push_back(cfg);
        }

        return true;
    }

    const std::vector<ChannelConfig> &GatewayConfig::getChannels() const
    {
        return channels_;
    }

} // namespace channel
