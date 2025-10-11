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
        GatewayConfig(std::vector<ChannelConfig> channels) : channels_(std::move(channels)) {};

        std::vector<ChannelConfig> getChannels() const
        {
            return channels_;
        }

    private:
        std::vector<ChannelConfig> channels_;
    };

} // namespace channel