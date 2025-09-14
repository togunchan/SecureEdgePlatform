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

            shell_.addScheduledSensor(args[0], std::stoull(args[1]));
        }

    private:
        sensor::EdgeShell &shell_;
    };
}