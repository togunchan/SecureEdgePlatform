#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include "../sensors/SimpleTempSensor.hpp"
#include "commands/CommandRegistry.hpp"

namespace sensor
{
    class EdgeShell
    {
    public:
        void run();
        void listSensors() const;

    private:
        void printHelp() const;
        void handleCommand(const std::string &line);
        void injectFault(const std::string &faultType, const std::string &sensorId);
        void resetSensor(const std::string &sensorId);
        void stepSensor(const std::string &sensorId);
        void addDefaultSensor();
        void addSensor(const std::string &sensorId);
        void stepAllSensors();

        std::unordered_map<std::string, std::unique_ptr<SimpleTempSensor>> sensors_;

        std::unique_ptr<cli::CommandRegistry> registry_;
    };

} // namespace sensor