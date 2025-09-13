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
                std::cout << "Usage: inject <type> <sensorId> [params...]\n";
                return;
            }

            const std::string &faultType = args[0];
            const std::string &sensorId = args[1];

            std::vector<std::string> params;
            if (args.size() > 2)
                params.assign(args.begin() + 2, args.end());

            shell_.injectFault(faultType, sensorId, params);
        }

    private:
        sensor::EdgeShell &shell_;
    };
}