#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include "../sensors/SimpleSensor.hpp"
#include "../scheduler/SensorScheduler.hpp"
#include "commands/CommandRegistry.hpp"
#include <thread>
#include <atomic>
#include <condition_variable>
#include "../../CppMiniDB/include/cppminidb/MiniDB.hpp"

namespace sensor
{
    enum class Mode
    {
        Full,      // all commands
        Restricted // only injection-related
    };

    class EdgeShell
    {
    public:
        void run(Mode mode = Mode::Full);
        void listSensors() const;
        void stepAllSensors();
        void stepSensor(const std::string &sensorId);
        void printHelp() const;
        void injectFault(const std::string &faultType, const std::string &sensorId, const std::vector<std::string> &params);
        void resetSensor(const std::string &sensorId);
        void addScheduledSensor(const std::string &sensorId, uint64_t period_ms);
        void tickTime(uint64_t delta_ms);
        void plotSensorData(const std::string &sensorId) const;
        void setDatabase(MiniDB *db);
        const std::unordered_map<std::string, std::unique_ptr<ISensor>> &getSensors() const;
        bool removeSensor(const std::string &id);
        void stop();
        void setScheduler(sensor::SensorScheduler *externalScheduler);

    private:
        void handleCommand(const std::string &line);
        void addDefaultSensor();
        sensor::SensorScheduler &activeScheduler()
        {
            return external_scheduler_ ? *external_scheduler_ : scheduler_;
        }
        const sensor::SensorScheduler &activeScheduler() const
        {
            return external_scheduler_ ? *external_scheduler_ : scheduler_;
        }

        std::unordered_map<std::string, std::unique_ptr<ISensor>> owned_sensors_;
        std::unique_ptr<cli::CommandRegistry> registry_;
        sensor::SensorScheduler scheduler_;
        std::atomic<bool> is_running_{false};
        std::thread run_thread_;
        std::atomic<bool> is_plotting_{false};
        std::thread plot_thread_;
        MiniDB *db_ = nullptr;
        std::condition_variable cv_;
        std::mutex cv_mutex_;
        Mode current_mode_{Mode::Full};
        sensor::SensorScheduler *external_scheduler_ = nullptr;
    };

} // namespace sensor
