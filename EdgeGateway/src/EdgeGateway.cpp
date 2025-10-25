#include <iostream>
#include <ConsoleChannel.hpp>
#include <FileChannel.hpp>
#include <cppminidb/SensorLogRow.hpp>
#include <EdgeGateway.hpp>
#include <memory>
#include <GatewayConfig.hpp>
#include <sensors/Spec.hpp>
#include <csignal>
#include <thread>
#include <AgentChannel.hpp>

namespace
{
    std::atomic<bool> keepRunning = true;
    void handleSignal(int signal)
    {
        if (signal == SIGINT)
        {
            std::cout << "\n[EdgeGateway] SIGINT received. Stopping loop...\n";
            keepRunning = false;
        }
    }
}

namespace gateway
{
    void EdgeGateway::start()
    {
        channel::GatewayConfig config(std::vector<channel::ChannelConfig>{});

        const std::string configPath = "EdgeGateway/data/gateway_config.json";

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
            else if (cfg.type == "agent")
            {
                std::cout << "[EdgeGateway] Adding AgentChannel...\n";
                channels_.push_back(std::make_unique<channel::AgentChannel>(&agent_));
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

        running_.store(false);
        scheduler_.onSample = [this](const cppminidb::SensorLogRow &row)
        {
            if (!running_.load())
            {
                return;
            }
            for (const auto &channel : channels_)
            {
                channel->publish(row);
            }
        };
        auto spec = sensor::makeDefaultTempSpec();
        spec.id = "TEMP-001";
        if (sensors_.find(spec.id) != sensors_.end())
        {
            std::cerr << "[EdgeGateway] Sensor already registered: " << spec.id << "\n";
            return;
        }

        auto sensor = std::make_unique<sensor::SimpleSensor>(spec);
        sensor::ISensor *sensorPtr = sensor.get();
        sensors_.emplace(spec.id, std::move(sensor));
        scheduler_.addScheduledSensor(spec.id, sensorPtr, 1000);
        std::cout << "Sensor added: " << spec.id << "\n";
    }

    void EdgeGateway::setChannelsForTest(std::unique_ptr<channel::IGatewayChannel> ch)
    {
        channels_.push_back(std::move(ch));
    }

    void EdgeGateway::setSampleCallbackForTest()
    {
        running_.store(true);
        scheduler_.onSample = [this](const cppminidb::SensorLogRow &row)
        {
            if (!running_.load())
            {
                return;
            }
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

    void EdgeGateway::runLoop()
    {
        std::signal(SIGINT, handleSignal);
        keepRunning = true;
        running_.store(true);
        std::cout << "[EdgeGateway] Starting run loop. Press Ctrl+C to exit.\n";
        uint64_t tick_interval_ms = 1000;
        while (keepRunning)
        {
            scheduler_.tick(tick_interval_ms);
            std::this_thread::sleep_for(std::chrono::milliseconds(tick_interval_ms));
        }
        running_.store(false);
    }

} // namespace gateway
