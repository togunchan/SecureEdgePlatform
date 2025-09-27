#pragma once
#include "ICommand.hpp"
#include "cli/EdgeShell.hpp"
#include <iostream>

using namespace sensor;

namespace cli
{
    class RemoveCommand : public ICommand
    {
    public:
        explicit RemoveCommand(EdgeShell &shell) : shell_(shell) {}

        std::string name() const override { return "remove"; }

        void execute(const std::vector<std::string> &args) override
        {
            if (args.size() < 2)
            {
                std::cout << "Usage: remove <id>\n";
                return;
            }

            const std::string &sensorId = args[1];
            bool removed = shell_.removeSensor(sensorId);
            if (removed)
            {
                std::cout << "Sensor removed: " << sensorId << "\n";
            }
            else
            {
                std::cout << "Sensor not found: " << sensorId << "\n";
            }
        }

    private:
        EdgeShell &shell_;
    };
}