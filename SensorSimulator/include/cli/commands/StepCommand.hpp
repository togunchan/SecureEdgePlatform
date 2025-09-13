#pragma once
#include "ICommand.hpp"
#include "../EdgeShell.hpp"

namespace cli
{
    class StepCommand : public ICommand
    {

    public:
        explicit StepCommand(sensor::EdgeShell &shell) : shell_(shell) {}

        std::string name() const override
        {
            return "step";
        }
        void execute(const std::vector<std::string> &args) override
        {
            if (args.empty())
            {
                std::cout << "Usage: step <sensor_id> or step all\n";
                return;
            }

            if (args[0] == "all")
            {
                shell_.stepAllSensors();
            }
            else
            {
                shell_.stepSensor(args[0]);
            }
        }

    private:
        sensor::EdgeShell &shell_;
    };
}