#pragma once
#include "ICommand.hpp"
#include "../EdgeShell.hpp"

namespace cli
{
    class HelpCommand : public ICommand
    {
    public:
        explicit HelpCommand(sensor::EdgeShell &shell) : shell_(shell)
        {
        }

        std::string name() const override
        {
            return "help";
        }

        void execute(const std::vector<std::string> &args) override
        {
            shell_.printHelp();
        }

    private:
        sensor::EdgeShell &shell_;
    };
}