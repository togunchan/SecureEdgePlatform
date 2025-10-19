#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <sensors/SimpleSensor.hpp>
#include <cppminidb/MiniDB.hpp>
#include <functional>
#include <cppminidb/SensorLogRow.hpp>

namespace sensor
{
    class SensorScheduler
    {
    public:
        // Adds a new sensor to the scheduler with a given sampling period (in ms)
        void addScheduledSensor(const std::string &id, ISensor *sensor, uint64_t period_ms);

        // Removes a sensor from the scheduler
        void removeSensor(const std::string &id);

        // Advances internal time and samples sensors whose time has come
        void tick(uint64_t delta_ms);

        // Lists current sensor IDs and their next sample time
        void listSensorStates() const;

        SimpleSensor *getScheduledSensor(const std::string &id) const;

        // Returns the current simulation time
        uint64_t getNow() const;

        void setDatabase(MiniDB *db);

        void removeScheduledSensor(const std::string &id);

        template <typename T>
        T *getScheduledSensorAs(const std::string &id) const
        {
            auto it = schedule_.find(id);
            if (it != schedule_.end())
            {
                return dynamic_cast<T *>(it->second.sensor);
            }
            return nullptr;
        }

        std::function<void(const cppminidb::SensorLogRow &)> onSample;

    private:
        struct SensorEntry
        {
            ISensor *sensor;
            uint64_t period_ms;
            uint64_t next_sample_time_ms;
        };

        uint64_t current_time_ms_ = 0;
        std::unordered_map<std::string, SensorEntry> schedule_;
        MiniDB *db_ = nullptr;
    };
}