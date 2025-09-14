#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include "../sensors/SimpleTempSensor.hpp"

namespace sensor
{
    class SensorScheduler
    {
    public:
        // Adds a new sensor to the scheduler with a given sampling period (in ms)
        void addScheduledSensor(const std::string &id, std::unique_ptr<SimpleTempSensor> sensor, uint64_t period_ms);

        // Removes a sensor from the scheduler
        void removeSensor(const std::string &id);

        // Advances internal time and samples sensors whose time has come
        void tick(uint64_t delta_ms);

        // Lists current sensor IDs and their next sample time
        void listSensorStates() const;

        SimpleTempSensor *getScheduledSensor(const std::string &id);

    private:
        struct SensorEntry
        {
            std::unique_ptr<SimpleTempSensor> sensor;
            uint64_t period_ms;
            uint64_t next_sample_time_ms;
        };

        uint64_t current_time_ms_ = 0;
        std::unordered_map<std::string, SensorEntry> schedule_;
    };
}