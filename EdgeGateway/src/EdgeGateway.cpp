#include <EdgeGateway.hpp>
#include <iostream>
#include <ConsoleChannel.hpp>
#include <FileChannel.hpp>
#include <cppminidb/SensorLogRow.hpp>
#include <EdgeGateway.hpp>
#include <memory>
#include <GatewayConfig.hpp>

namespace gateway
{
    void EdgeGateway::start()
    {
        channel::GatewayConfig config(std::vector<channel::ChannelConfig>{});

        const std::string configPath = "../EdgeGateway/data/gateway_config.json";

        if (!config.loadFromFile(configPath))
        {
            std::cerr << "[EdgeGateway] Failed to load gateway configuration.\n";
            return;
        }

        const auto &channels = config.getChannels();
        std::cout << "Loaded Channels:\n";
        for (const auto &ch : channels)
        {
            std::cout << "- Type: " << ch.type << ", Path: " << ch.path << "\n";
        }

        for (const auto &cfg : channels)
        {
            if (cfg.type == "console")
            {
                channels_.push_back(std::make_unique<channel::ConsoleChannel>());
            }
            else if (cfg.type == "file")
            {
                if (cfg.path.empty())
                {
                    std::cerr << "[EdgeGateway] File channel requires a 'path' in the configuration.\n";
                    continue;
                }
                channels_.push_back(std::make_unique<channel::FileChannel>(cfg.path));
            }
            else
            {
                std::cerr << "[EdgeGateway] Unknown channel type: " << cfg.type << "\n";
            }
        }

        if (channels_.empty())
        {
            std::cerr << "[EdgeGateway] No active channels configured.\n";
            return;
        }

        cppminidb::SensorLogRow sampleRow;
        sampleRow.timestamp_ms = 1760930400000;
        sampleRow.sensor_id = "temp_01";
        sampleRow.value = 24.7;

        for (const auto &channel : channels_)
        {
            channel->publish(sampleRow);
        }
    }

} // namespace gateway