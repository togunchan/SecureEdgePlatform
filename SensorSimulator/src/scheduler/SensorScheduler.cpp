#include <iostream>
#include "../../include/scheduler/SensorScheduler.hpp"

namespace sensor
{
    void SensorScheduler::addScheduledSensor(const std::string &id, ISensor *sensor, uint64_t period_ms)
    {
        if (schedule_.count(id))
        {
            std::cout << "Sensor already exists: " << id << "\n";
            return;
        }

        SensorEntry entry;
        entry.sensor = sensor;
        entry.period_ms = period_ms;
        entry.next_sample_time_ms = current_time_ms_;
        schedule_[id] = std::move(entry);
        std::cout << "Sensor scheduled: " << id << " (period: " << period_ms << " ms)\n";

        for (auto &[id, entry] : schedule_)
        {
            std::cout << "  " << id << " (period: " << entry.period_ms << " ms, next at: " << entry.next_sample_time_ms << " ms)\n";
        }
    }

    void SensorScheduler::removeSensor(const std::string &id)
    {
        if (schedule_.erase(id))
        {
            std::cout << "Sensor removed: " << id << "\n";
        }
        else
        {
            std::cout << "Sensor not found: " << id << "\n";
        }
    }

    void SensorScheduler::tick(uint64_t delta_ms)
    {
        current_time_ms_ += delta_ms;

        for (auto &[id, entry] : schedule_)
        {
            if (!entry.sensor)
            {
                std::cerr << "NULL sensor for id=" << id << "\n";
                continue;
            }
            if (getNow() > entry.next_sample_time_ms)
            {
                auto sample = entry.sensor->nextSample(getNow());
                std::cout << "[Tick @ " << getNow() << "]  "
                          << "Sensor " << entry.sensor->id() << " → value: " << sample.value << "\n";
                entry.next_sample_time_ms += entry.period_ms;

                if (db_)
                {
                    auto faults = entry.sensor->getActiveFaults(getNow());
                    db_->appendLog(id, getNow(), sample.value, faults);
                }

                if (onSample)
                {
                    cppminidb::SensorLogRow row;
                    row.timestamp_ms = getNow();
                    row.sensor_id = id;
                    row.value = sample.value;
                    row.fault_flags = entry.sensor->getActiveFaults(getNow());

                    onSample(row);
                }
            }
        }
    }

    void SensorScheduler::listSensorStates() const
    {
        std::cout << "Scheduled Sensors:\n";
        for (const auto &[id, entry] : schedule_)
        {
            std::cout << "  " << id << " (period: " << entry.period_ms
                      << " ms, next at: " << entry.next_sample_time_ms << " ms)\n";
        }
    }

    SimpleSensor *SensorScheduler::getScheduledSensor(const std::string &id) const
    {
        auto it = schedule_.find(id);
        if (it != schedule_.end())
        {
            return dynamic_cast<SimpleSensor *>(it->second.sensor);
        }
        return nullptr;
    }

    uint64_t SensorScheduler::getNow() const
    {
        return current_time_ms_;
    }

    void SensorScheduler::setDatabase(MiniDB *db)
    {
        db_ = db;
    }

    void SensorScheduler::removeScheduledSensor(const std::string &id)
    {
        auto it = schedule_.find(id);
        if (it != schedule_.end())
        {
            schedule_.erase(it);
            std::cout << "Sensor unscheduled: " << id << "\n";
        }
    }

}