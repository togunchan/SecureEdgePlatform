#pragma once
#include "ICommand.hpp"
#include "../EdgeShell.hpp"
#include <cstdlib>

namespace cli
{
    class TickCommand : public ICommand
    {
    public:
        explicit TickCommand(sensor::EdgeShell &shell) : shell_(shell) {}

        std::string name() const override
        {
            return "tick";
        }

        void execute(const std::vector<std::string> &args) override
        {
            if (args.empty())
            {
                std::cout << "Usage: tick <delta_ms>\n";
                return;
            }
            std::cout << "I am in execute function" << std::endl;

            uint64_t delta = std::strtoull(args[0].c_str(), nullptr, 10);
            shell_.tickTime(delta);
        }

    private:
        sensor::EdgeShell &shell_;
    };
}