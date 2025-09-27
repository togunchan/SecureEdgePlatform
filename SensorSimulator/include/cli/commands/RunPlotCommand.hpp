#pragma once

#include "ICommand.hpp"
#include "../EdgeShell.hpp"
#include <unordered_map>
#include <thread>

namespace cli
{
    class RunPlotCommand : public ICommand
    {
    public:
        RunPlotCommand(sensor::EdgeShell &edgeShell,
                       std::atomic<bool> &isPlotting,
                       std::thread &plotThread)
            : edgeShell_(edgeShell),
              isPlotting_(isPlotting),
              plotThread_(plotThread) {}

        std::string name() const override
        {
            return "runplot";
        }

        void execute(const std::vector<std::string> &args) override
        {
            if (isPlotting_)
            {
                std::cout << "Plotting is already doing its job...\n";
                return;
            }

            sensorId_ = args[0];

            isPlotting_ = true;
            plotThread_ = std::thread([this]()
                                      {
                                        while(isPlotting_){
                                            std::cout << "\n";
                                            edgeShell_.plotSensorData(sensorId_);
                                            std::this_thread::sleep_for(std::chrono::milliseconds(500));
                                        } });
            std::cout << "Started real-time plotting of " << sensorId_ << ". Use 'stopplot' to stop.\n";
        }

    private:
        sensor::EdgeShell &edgeShell_;
        std::atomic<bool> &isPlotting_;
        std::thread &plotThread_;
        std::string sensorId_;
    };
}