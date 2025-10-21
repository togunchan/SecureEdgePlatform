#include <iostream>
#include <ConsoleChannel.hpp>
#include <FileChannel.hpp>
#include <cppminidb/SensorLogRow.hpp>
#include <EdgeGateway.hpp>
#include <memory>
#include <GatewayConfig.hpp>
#include <sensors/Spec.hpp>

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

        scheduler_.onSample = [this](const cppminidb::SensorLogRow &row)
        {
            for (const auto &channel : channels_)
            {
                channel->publish(row);
            }
        };
        auto spec = sensor::makeDefaultTempSpec();
        spec.id = "TEMP-001";
        auto sensor = std::make_unique<sensor::SimpleSensor>(spec);
        sensor::ISensor *sensorPtr = sensor.get();
        scheduler_.addScheduledSensor(spec.id, sensorPtr, 1000);
        std::cout << "Sensor added: " << spec.id << "\n";
        scheduler_.tick(1000);
    }

    void EdgeGateway::setChannelsForTest(std::unique_ptr<channel::IGatewayChannel> ch)
    {
        channels_.push_back(std::move(ch));
    }

    void EdgeGateway::setSampleCallbackForTest()
    {
        scheduler_.onSample = [this](const cppminidb::SensorLogRow &row)
        {
            for (const auto &channel : channels_)
            {
                channel->publish(row);
            }
        };
    }

    void EdgeGateway::injectTestSample(const cppminidb::SensorLogRow &row)
    {
        if (scheduler_.onSample)
            scheduler_.onSample(row);
    }

} // namespace gateway