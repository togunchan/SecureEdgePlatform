#pragma once
#include "ICommand.hpp"
#include "../EdgeShell.hpp"

namespace cli
{
    class PlotCommand : public ICommand
    {
    public:
        explicit PlotCommand(sensor::EdgeShell &shell) : shell_(shell) {}

        std::string name() const override { return "plot"; }

        void execute(const std::vector<std::string> &args) override
        {
            if (args.size() != 1 || args.empty())
            {
                std::cout << "Usage: plot <sensorId>\n";
                return;
            }

            shell_.plotSensorData(args[0]);
        }

    private:
        sensor::EdgeShell &shell_;
    };
}