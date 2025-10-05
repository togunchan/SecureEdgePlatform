#include "secureboot/BootStage.hpp"
#include <chrono>
#include <iostream>

namespace secureboot
{
    BootStage::BootStage(std::string name, int order, std::function<void()> onSimulate)
        : name_(std::move(name)), order_(order), onSimulate_(std::move(onSimulate)) {}

    void BootStage::simulate()
    {
        using namespace std::chrono;

        auto start = high_resolution_clock::now();
        try
        {
            if (onSimulate_)
            {
                onSimulate_();
            }
            success_ = true;
        }
        catch (const std::exception &e)
        {
            success_ = false;
            errorCode_ = -1;
            std::cerr << "[BootStage::simulate] Exception: " << e.what() << '\n';
        }
        catch (...)
        {
            success_ = false;
            errorCode_ = -999;
            std::cerr << "[BootStage::simulate] Unknown exception occurred.\n";
        }

        auto end = high_resolution_clock::now();
        durationMs_ = duration_cast<milliseconds>(end - start).count();
    }

    const std::string &BootStage::getName() const
    {
        return name_;
    }

    int BootStage::getOrder() const
    {
        return order_;
    }

    bool BootStage::wasSuccessful() const
    {
        return success_;
    }

    std::optional<int> BootStage::getErrorCode() const
    {
        return errorCode_;
    }

    std::optional<long long> BootStage::getDurationMs() const
    {
        return durationMs_;
    }
} // namespace secureboot
