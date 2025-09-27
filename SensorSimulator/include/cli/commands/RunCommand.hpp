#pragma once

#include "ICommand.hpp"
#include <unordered_map>
#include "scheduler/SensorScheduler.hpp"
#include <thread>

namespace cli
{
    class RunCommand : public ICommand
    {
    public:
        explicit RunCommand(sensor::SensorScheduler &scheduler, std::atomic<bool> &is_running, std::thread &run_thread, std::condition_variable &cv, std::mutex &cv_mutex) : scheduler_(scheduler), is_running_(is_running), run_thread_(run_thread), cv_(cv), cv_mutex_(cv_mutex) {}

        std::string name() const override
        {
            return "run";
        }
        void execute(const std::vector<std::string> &args) override
        {
            if (is_running_)
            {
                std::cout << "Simulation is already running.\n";
                return;
            }

            is_running_ = true;
            run_thread_ = std::thread([this]()
                                      {
                                        const int tick_ms = 1000;
                                        std::unique_lock<std::mutex> lock(cv_mutex_);
                                        while (is_running_)
                                        {
                                            scheduler_.tick(tick_ms);
                                            cv_.wait_for(lock, std::chrono::seconds(1), [this]() { return !is_running_; });
                                        } });
            std::cout << "Started real-time simulation. Use 'stop' to halt.\n";
        }

    private:
        sensor::SensorScheduler &scheduler_;
        std::atomic<bool> &is_running_;
        std::thread &run_thread_;
        std::condition_variable &cv_;
        std::mutex &cv_mutex_;
    };
}