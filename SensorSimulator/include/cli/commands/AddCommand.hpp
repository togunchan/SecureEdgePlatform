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
                std::cout << "add <id>" << std::endl;
                return;
            }

            shell_.addSensor(args[0]);
        }

    private:
        sensor::EdgeShell &shell_;
    };
}