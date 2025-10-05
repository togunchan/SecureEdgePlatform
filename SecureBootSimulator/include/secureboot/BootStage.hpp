#pragma once

#include <string>
#include <functional>
#include <chrono>
#include <optional>

namespace secureboot
{
    class BootStage
    {
    public:
        BootStage(std::string name, int order, std::function<void()> onSimulate = nullptr);

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

        bool success_{false};
        std::optional<int> errorCode_;
        std::optional<long long> durationMs_;
    };

} // namespace secureboot