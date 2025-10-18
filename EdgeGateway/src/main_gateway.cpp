#include <GatewayConfig.hpp>
#include <IGatewayChannel.hpp>
#include <ConsoleChannel.hpp>
#include <FileChannel.hpp>
#include <cppminidb/SensorLogRow.hpp>
#include <memory>
#include <iostream>

int main()
{
    channel::GatewayConfig config(std::vector<channel::ChannelConfig>{});

    const std::string configPath = "../EdgeGateway/data/gateway_config.json";

    if (!config.loadFromFile(configPath))
    {
        std::cerr << "[EdgeGateway] Failed to load gateway configuration.\n";
        return 1;
    }

    const auto &channels = config.getChannels();
    std::cout << "Loaded Channels:\n";
    for (const auto &ch : channels)
    {
        std::cout << "- Type: " << ch.type << ", Path: " << ch.path << "\n";
    }

    std::vector<std::unique_ptr<channel::IGatewayChannel>> activeChannels;

    for (const auto &cfg : channels)
    {
        if (cfg.type == "console")
        {
            activeChannels.push_back(std::make_unique<channel::ConsoleChannel>());
        }
        else if (cfg.type == "file")
        {
            if (cfg.path.empty())
            {
                std::cerr << "[EdgeGateway] File channel requires a 'path' in the configuration.\n";
                continue;
            }
            activeChannels.push_back(std::make_unique<channel::FileChannel>(cfg.path));
        }
        else
        {
            std::cerr << "[EdgeGateway] Unknown channel type: " << cfg.type << "\n";
        }
    }

    if (activeChannels.empty())
    {
        std::cerr << "[EdgeGateway] No active channels configured.\n";
        return 1;
    }

    cppminidb::SensorLogRow sampleRow;
    sampleRow.timestamp_ms = 1760930400000;
    sampleRow.sensor_id = "temp_01";
    sampleRow.value = 24.7;

    for (const auto &channel : activeChannels)
    {
        channel->publish(sampleRow);
    }

    return 0;
}
