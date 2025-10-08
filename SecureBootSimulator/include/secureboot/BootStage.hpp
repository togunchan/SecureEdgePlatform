#pragma once

#include <chrono>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>

namespace secureboot
{
    class StageFailure : public std::runtime_error
    {
    public:
        StageFailure(int errorCode, const std::string &message);

        int errorCode() const noexcept;

    private:
        int errorCode_;
    };

    class BootStage
    {
    public:
        BootStage(std::string name, int order, std::function<void()> onSimulate = nullptr, std::function<void()> onFail = nullptr);

        static constexpr int kMissingHandlerErrorCode = -1;
        static constexpr int kUnhandledExceptionErrorCode = -2;

        void simulate();

        const std::string &getName() const;
        int getOrder() const;
        bool wasSuccessful() const;
        std::optional<int> getErrorCode() const;
        std::optional<long long> getDurationMs() const;

    private:
        std::string name_;
        int order_;
        std::function<void()> onSimulate_;
        std::function<void()> onFail_;

        bool success_{false};
        std::optional<int> errorCode_;
        std::optional<long long> durationMs_;
    };

} // namespace secureboot
