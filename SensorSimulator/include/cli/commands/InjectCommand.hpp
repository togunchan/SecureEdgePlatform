#pragma once
#include "ICommand.hpp"
#include "../EdgeShell.hpp"

namespace cli
{
    class InjectCommand : public ICommand
    {
    public:
        explicit InjectCommand(sensor::EdgeShell &shell) : shell_(shell) {}

        std::string name() const override
        {
            return "inject";
        }

        void execute(const std::vector<std::string> &args) override
        {
            if (args.size() < 2)
            {
                std::cout << "inject <type> <id>       - Inject fault (spike/stuck/dropout)\n";
                return;
            }
            shell_.injectFault(args[0], args[1]);
        }

    private:
        sensor::EdgeShell &shell_;
    };
}