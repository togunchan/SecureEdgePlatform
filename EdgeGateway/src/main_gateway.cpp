#include <GatewayConfig.hpp>
#include <iostream>

int main()
{
    channel::GatewayConfig config(std::vector<channel::ChannelConfig>{});

    const std::string configPath = "../EdgeGateway/data/gateway_config.json";

    if (!config.loadFromFile(configPath))
    {
        std::cerr << "Failed to load gateway configuration.\n";
        return 1;
    }

    const auto &channels = config.getChannels();
    std::cout << "Loaded Channels:\n";
    for (const auto &ch : channels)
    {
        std::cout << "- Type: " << ch.type << ", Path: " << ch.path << "\n";
    }

    return 0;
}
