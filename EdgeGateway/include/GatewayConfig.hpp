#pragma once

#include <string>
#include <vector>

namespace channel
{
    struct ChannelConfig
    {
        std::string type; // e.g., "console", "file"
        std::string path; // only for file channels
    };

    class GatewayConfig
    {
    public:
        GatewayConfig() = default;
        GatewayConfig(std::vector<ChannelConfig> channels) : channels_(std::move(channels)) {};

        bool loadFromFile(const std::string &path);

        const std::vector<ChannelConfig> &getChannels() const;

    private:
        std::vector<ChannelConfig> channels_;
    };

} // namespace channel