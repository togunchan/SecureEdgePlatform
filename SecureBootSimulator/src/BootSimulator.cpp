#include "secureboot/BootSimulator.hpp"

#include <algorithm>
#include <iostream>
#include <string>

namespace secureboot
{
    BootSimulator::BootSimulator(const BootConfig &config, HashMethod hashMethod)
        : config_(std::move(config)), verifier_(hashMethod) {}

    void BootSimulator::addStage(const BootStage &stage)
    {
        stages_.push_back(stage);
    }

    bool BootSimulator::verifyFirmware()
    {
        std::cout << "[SecureBoot] Verifying firmware signature..." << std::endl;
        try
        {
            const std::string actualHash = verifier_.computeHash(config_.getFirmwarePath());
            const std::string expectedHash = config_.getExpectedSha256();

            if (!verifier_.compareHash(actualHash, expectedHash))
            {
                failureReason_ = "Firmware signature mismatch.";
                return false;
            }
        }
        catch (const std::exception &ex)
        {
            failureReason_ = std::string("Firmware verification error: ") + ex.what();
            return false;
        }

        return true;
    }

    void BootSimulator::run()
    {
        std::cout << "[SecureBoot] Starting boot process...\n"
                  << std::endl;

        failureReason_.clear();
        success_ = false;

        if (!verifyFirmware())
        {
            std::cerr << "[SecureBoot] Verification failed: " << failureReason_ << std::endl;
            return;
        }

        if (stages_.empty())
        {
            failureReason_ = "No boot stages configured.";
            std::cerr << "[SecureBoot] " << failureReason_ << std::endl;
            return;
        }

        std::stable_sort(stages_.begin(), stages_.end(), [](const BootStage &lhs, const BootStage &rhs)
                         { return lhs.getOrder() < rhs.getOrder(); });

        for (auto &stage : stages_)
        {
            std::cout << "[SecureBoot] ➤ Executing stage: " << stage.getName() << std::endl;
            stage.simulate();

            if (!stage.wasSuccessful())
            {
                const auto errorCode = stage.getErrorCode();
                if (errorCode.has_value())
                {
                    failureReason_ = "Stage '" + stage.getName() + "' failed with error code: " + std::to_string(errorCode.value());
                }
                else
                {
                    failureReason_ = "Stage '" + stage.getName() + "' failed with an unknown error.";
                }
                std::cerr << "[SecureBoot] " << failureReason_ << std::endl;
                return;
            }

            const auto duration = stage.getDurationMs();
            if (duration.has_value())
            {
                std::cout << "[SecureBoot] Stage '" << stage.getName() << "' completed in " << duration.value() << " ms\n"
                          << std::endl;
            }
            else
            {
                std::cout << "[SecureBoot] Stage '" << stage.getName() << "' completed.\n"
                          << std::endl;
            }
        }
        success_ = true;
        std::cout << "[SecureBoot] Boot process completed successfully.\n"
                  << std::endl;
    }

    bool BootSimulator::wasSuccessful() const
    {
        return success_;
    }

    std::string BootSimulator::getFailureReason() const
    {
        return failureReason_;
    }

} // namespace secureboot
