#include <iostream>
#include "../../include/scheduler/SensorScheduler.hpp"
#include "../../include/cli/EdgeShell.hpp"

namespace sensor
{
    void SensorScheduler::addScheduledSensor(const std::string &id, std::unique_ptr<SimpleSensor> sensor, uint64_t period_ms)
    {
        if (schedule_.count(id))
        {
            std::cout << "Sensor already exists: " << id << "\n";
            return;
        }

        SensorEntry entry;
        entry.sensor = std::move(sensor);
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
            if (getNow() > entry.next_sample_time_ms)
            {
                auto sample = entry.sensor->nextSample(getNow());
                std::cout << "[Tick @ " << getNow() << "]  " << entry.sensor.get()->getSpec().id << " → value: " << sample.value << "\n";
                entry.next_sample_time_ms += entry.period_ms;

                if (db_)
                {
                    auto faults = entry.sensor->getActiveFaults(getNow());
                    db_->appendLog(id, getNow(), sample.value, faults);
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
            return it->second.sensor.get();
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

}