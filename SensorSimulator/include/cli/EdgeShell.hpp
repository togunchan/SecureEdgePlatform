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
        void stepAllSensors();
        void stepSensor(const std::string &sensorId);
        void printHelp() const;
        void injectFault(const std::string &faultType, const std::string &sensorId);
        void resetSensor(const std::string &sensorId);
        void addSensor(const std::string &sensorId);

    private:
        void handleCommand(const std::string &line);
        void addDefaultSensor();

        std::unordered_map<std::string, std::unique_ptr<SimpleTempSensor>> sensors_;

        std::unique_ptr<cli::CommandRegistry> registry_;
    };

} // namespace sensor