#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include "../sensors/SimpleTempSensor.hpp"

namespace sensor
{
    class EdgeShell
    {
    public:
        void run();

    private:
        void printHelp() const;
        void handleCommand(const std::string &line);
        void injectFault(const std::string &faultType, const std::string &sensorId);
        void resetSensor(const std::string &sensorId);
        void stepSensor(const std::string &sensorId);
        void listSensors() const;
        void addDefaultSensor();

        std::unordered_map<std::string, std::unique_ptr<SimpleTempSensor>> sensors_;
    };

} // namespace sensor