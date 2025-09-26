#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include "../sensors/SimpleSensor.hpp"
#include "../scheduler/SensorScheduler.hpp"
#include "commands/CommandRegistry.hpp"
#include <thread>
#include <atomic>
#include "../../CppMiniDB/include/cppminidb/MiniDB.hpp"

namespace sensor
{
    class EdgeShell
    {
    public:
        void run();
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

    private:
        void handleCommand(const std::string &line);
        void addDefaultSensor();

        std::unordered_map<std::string, std::unique_ptr<ISensor>> sensors_;
        std::unique_ptr<cli::CommandRegistry> registry_;
        sensor::SensorScheduler scheduler_;
        std::atomic<bool> is_running_{false};
        std::thread run_thread_;
        std::atomic<bool> is_plotting_{false};
        std::thread plot_thread_;
        MiniDB *db_ = nullptr;
    };

} // namespace sensor