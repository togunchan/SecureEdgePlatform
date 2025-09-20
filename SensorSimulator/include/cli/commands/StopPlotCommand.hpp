#pragma once

#include "ICommand.hpp"
#include "../EdgeShell.hpp"
#include <unordered_map>
#include <thread>

namespace cli
{
    class StopPlotCommand : public ICommand
    {
    public:
        StopPlotCommand(std::atomic<bool> &isPlotting, std::thread &plotThread)
            : isPlotting_(isPlotting), plotThread_(plotThread) {}

        std::string name() const override
        {
            return "stopplot";
        }

        void execute(const std::vector<std::string> &args) override
        {
            if (!isPlotting_)
            {
                std::cout << "Plotting is not active.\n";
                return;
            }

            isPlotting_ = false;
            if (plotThread_.joinable())
            {
                plotThread_.join();
            }

            std::cout << "Stopped plotting.\n";
        }

    private:
        std::atomic<bool> &isPlotting_;
        std::thread &plotThread_;
    };
}
