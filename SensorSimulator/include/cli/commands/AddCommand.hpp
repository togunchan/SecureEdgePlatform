#pragma once
#include "ICommand.hpp"
#include "../EdgeShell.hpp"

namespace cli
{
    class AddCommand : public ICommand
    {
    public:
        explicit AddCommand(sensor::EdgeShell &shell) : shell_(shell) {}

        std::string name() const override
        {
            return "add";
        }

        void execute(const std::vector<std::string> &args) override
        {
            if (args.empty())
            {
                std::cout << "add <id> <period_ms>" << std::endl;
                return;
            }
            std::string sensorId = args[0];
            std::transform(sensorId.begin(), sensorId.end(), sensorId.begin(), ::toupper);

            uint64_t period_ms = 1000;

            if (args.size() > 1)
            {
                try
                {
                    period_ms = std::stoull(args[1]);
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Invalid period value: " << args[1] << "\n";
                    return;
                }
            }

            shell_.addScheduledSensor(sensorId, period_ms);
        }

    private:
        sensor::EdgeShell &shell_;
    };
}