#include "secureboot/BootStage.hpp"

#include <chrono>
#include <iostream>

namespace secureboot
{
    StageFailure::StageFailure(int errorCode, const std::string &message)
        : std::runtime_error(message), errorCode_(errorCode)
    {
    }

    int StageFailure::errorCode() const noexcept
    {
        return errorCode_;
    }

    BootStage::BootStage(std::string name, int order, std::function<void()> onSimulate, std::function<void()> onFail)
        : name_(std::move(name)),
          order_(order),
          onSimulate_(std::move(onSimulate)),
          onFail_(std::move(onFail)) {}

    void BootStage::simulate()
    {
        using namespace std::chrono;

        auto start = high_resolution_clock::now();
        success_ = false;
        errorCode_.reset();
        durationMs_.reset();

        auto invokeFailCallback = [this]()
        {
            if (onFail_)
            {
                onFail_();
            }
        };

        if (!onSimulate_)
        {
            errorCode_ = kMissingHandlerErrorCode;
            invokeFailCallback();
            durationMs_ = duration_cast<milliseconds>(high_resolution_clock::now() - start).count();
            return;
        }

        try
        {
            onSimulate_();
            success_ = true;
            errorCode_.reset();
        }
        catch (const StageFailure &failure)
        {
            errorCode_ = failure.errorCode();
            std::cerr << "[BootStage::simulate] Stage '" << name_ << "' failed: " << failure.what() << '\n';
            invokeFailCallback();
        }
        catch (const std::exception &ex)
        {
            errorCode_ = kUnhandledExceptionErrorCode;
            std::cerr << "[BootStage::simulate] Stage '" << name_ << "' threw exception: " << ex.what() << '\n';
            invokeFailCallback();
        }
        catch (...)
        {
            success_ = false;
            errorCode_ = kUnhandledExceptionErrorCode;
            std::cerr << "[BootStage::simulate] Stage '" << name_ << "' threw an unknown exception.\n";
            invokeFailCallback();
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
