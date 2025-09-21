#pragma once
#include "ICommand.hpp"
#include "../EdgeShell.hpp"

namespace cli
{
    class ResetCommand : public ICommand
    {
    public:
        explicit ResetCommand(sensor::EdgeShell &shell) : shell_(shell) {}

        std::string name() const override
        {
            return "reset";
        }

        void execute(const std::vector<std::string> &args) override
        {
            if (args.empty())
            {
                std::cout << "reset <id>" << std::endl;
                return;
            }

            shell_.resetSensor(args[0]);
        }

    private:
        sensor::EdgeShell &shell_;
    };
}