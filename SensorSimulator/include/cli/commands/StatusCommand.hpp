#pragma once

#include "ICommand.hpp"
#include "scheduler/SensorScheduler.hpp"
#include "sensors/SimpleSensor.hpp"
#include <iostream>

namespace cli
{
    class StatusCommand : public ICommand
    {
    public:
        explicit StatusCommand(const sensor::SensorScheduler &scheduler)
            : scheduler_(scheduler) {}

        std::string name() const override
        {
            return "status";
        }

        void execute(const std::vector<std::string> &args) override
        {
            if (args.size() != 1)
            {
                std::cout << "Usage: status <sensor_id>\n";
                return;
            }

            const std::string &sensorId = args[0];
            auto sensor = scheduler_.getScheduledSensor(sensorId);
            if (!sensor)
            {
                std::cout << "Sensor not found: " << sensorId << "\n";
                return;
            }

            auto faults = sensor->getActiveFaults(scheduler_.getNow());
            if (faults.empty())
            {
                std::cout << "Status: Normal - No active faults on " << sensorId << "\n";
            }
            else
            {
                for (const auto &f : faults)
                {
                    if (f == "spike")
                    {
                        std::cout << " * spike — transient fault active until "
                                  << sensor->getActiveSpike().end_time_ms << " ms\n";
                    }
                    else if (f == "stuck")
                    {
                        std::cout << " * stuck — output fixed until "
                                  << sensor->getActiveStuck().end_time_ms << " ms\n";
                    }
                    else if (f == "dropout")
                    {
                        std::cout << " * dropout — signal lost (NaN) until "
                                  << sensor->getActiveDropout().end_time_ms << " ms\n";
                    }
                }
            }
        }

    private:
        const sensor::SensorScheduler &scheduler_;
    };
}
