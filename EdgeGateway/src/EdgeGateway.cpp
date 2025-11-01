#include <iostream>
#include <ConsoleChannel.hpp>
#include <FileChannel.hpp>
#include <cppminidb/SensorLogRow.hpp>
#include <EdgeGateway.hpp>
#include <memory>
#include <GatewayConfig.hpp>
#include <sensors/Spec.hpp>
#include <sensors/SimpleSensor.hpp>
#include <thread>
#include <AgentChannel.hpp>
#include <filesystem>

namespace
{
    std::atomic<bool> keepRunning{false};
}

namespace gateway
{
    void EdgeGateway::start(const std::string &configPath)
    {
        channel::GatewayConfig config(std::vector<channel::ChannelConfig>{});

        channels_.clear();

        std::filesystem::path pathToUse = configPath.empty()
                                              ? (std::filesystem::path(__FILE__).parent_path().parent_path() / "data" / "gateway_config.json")
                                              : std::filesystem::path(configPath);

        if (!config.loadFromFile(pathToUse.string()))
        {
            std::cerr << "[EdgeGateway] Failed to load gateway configuration from: " << pathToUse.string() << "\n";
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

        const std::string defaultSensorId = "TEMP-001";
        if (!scheduler_.getScheduledSensor(defaultSensorId))
        {
            auto spec = sensor::makeDefaultTempSpec();
            spec.id = defaultSensorId;

            auto owned = sensors_.find(defaultSensorId);
            sensor::ISensor *sensorPtr = nullptr;
            if (owned == sensors_.end())
            {
                auto sensor = std::make_unique<sensor::SimpleSensor>(spec);
                sensorPtr = sensor.get();
                sensors_.emplace(defaultSensorId, std::move(sensor));
            }
            else
            {
                sensorPtr = owned->second.get();
            }

            scheduler_.addScheduledSensor(defaultSensorId, sensorPtr, 1000);
            std::cout << "[EdgeGateway] Scheduled default sensor: " << defaultSensorId << "\n";
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
        // auto spec = sensor::makeDefaultTempSpec();
        // spec.id = "TEMP-001";
        // if (scheduler_.getScheduledSensor(spec.id))
        // {
        //     std::cout << "[EdgeGateway] Sensor already scheduled: " << spec.id << "\n";
        //     return;
        // }

        // if (sensors_.find(spec.id) != sensors_.end())
        // {
        //     std::cerr << "[EdgeGateway] Sensor already registered: " << spec.id << "\n";
        //     return;
        // }

        // auto sensor = std::make_unique<sensor::SimpleSensor>(spec);
        // sensor::ISensor *sensorPtr = sensor.get();
        // sensors_.emplace(spec.id, std::move(sensor));
        // scheduler_.addScheduledSensor(spec.id, sensorPtr, 1000);
        // std::cout << "Sensor added: " << spec.id << "\n";
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
        if (keepRunning.exchange(true))
        {
            std::cerr << "[EdgeGateway] runLoop already active. Ignoring duplicate start request.\n";
            return;
        }
        running_.store(true, std::memory_order_release);
        std::cout << "[EdgeGateway] Starting run loop. Press Ctrl+C to exit." << std::endl;
        const uint64_t tick_interval_ms = 1000;
        while (keepRunning.load(std::memory_order_acquire))
        {
            scheduler_.tick(tick_interval_ms);

            std::this_thread::sleep_for(std::chrono::milliseconds(tick_interval_ms));
        }
        running_.store(false, std::memory_order_release);
        keepRunning.store(false, std::memory_order_release);
        std::cout << "[EdgeGateway] Run loop stopped.\n";
    }

    void EdgeGateway::stopLoop()
    {
        if (keepRunning.exchange(false))
        {
            std::cout << "[EdgeGateway] Stop requested. Waiting for loop to exit...\n";
        }
    }

    sensor::SensorScheduler &EdgeGateway::getScheduler()
    {
        return scheduler_;
    }

    const sensor::SensorScheduler &EdgeGateway::getScheduler() const
    {
        return scheduler_;
    }

} // namespace gateway
