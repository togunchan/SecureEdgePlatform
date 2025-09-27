
#include "ICommand.hpp"
#include <unordered_map>
#include "scheduler/SensorScheduler.hpp"
#include <thread>

namespace cli
{
    class StopCommand : public ICommand
    {
    public:
        StopCommand(sensor::EdgeShell &shell) : shell_(shell) {}

        std::string name() const override
        {
            return "stop";
        }

        void execute(const std::vector<std::string> &) override
        {
            shell_.stop();
        }

    private:
        sensor::EdgeShell &shell_;
    };
}