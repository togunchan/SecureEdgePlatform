#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include "../sensors/SimpleTempSensor.hpp"
#include "../scheduler/SensorScheduler.hpp"
#include "commands/CommandRegistry.hpp"

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
        void addSensor(const std::string &sensorId);
        void tickTime(uint64_t delta_ms);

    private:
        void handleCommand(const std::string &line);
        void addDefaultSensor();

        std::unordered_map<std::string, std::unique_ptr<SimpleTempSensor>> sensors_;
        std::unique_ptr<cli::CommandRegistry> registry_;
        sensor::SensorScheduler scheduler_;
    };

} // namespace sensor