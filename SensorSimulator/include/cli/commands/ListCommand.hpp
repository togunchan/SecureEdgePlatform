#pragma once
#include "ICommand.hpp"
#include "../EdgeShell.hpp"

namespace cli
{
    class ListCommand : public ICommand
    {
    public:
        explicit ListCommand(sensor::EdgeShell &shell) : shell_(shell) {}

        std::string name() const override
        {
            return "list";
        }
        void execute(const std::vector<std::string> &args) override
        {
            shell_.listSensors();
        }

    private:
        sensor::EdgeShell &shell_;
    };
}