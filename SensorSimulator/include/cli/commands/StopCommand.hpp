
#include "ICommand.hpp"
#include <unordered_map>
#include "scheduler/SensorScheduler.hpp"
#include <thread>

namespace cli
{
    class StopCommand : public ICommand
    {
    public:
        StopCommand(std::atomic<bool> &isRunning, std::thread &runThread)
            : isRunning_(isRunning), runThread_(runThread) {}

        std::string name() const override
        {
            return "stop";
        }

        void execute(const std::vector<std::string> &) override
        {
            if (!isRunning_)
            {
                std::cout << "Simulation is not running.\n";
                return;
            }

            isRunning_ = false;
            if (runThread_.joinable())
            {
                runThread_.join();
            }

            std::cout << "Simulation stopped.\n";
        }

    private:
        std::atomic<bool> &isRunning_;
        std::thread &runThread_;
    };
}