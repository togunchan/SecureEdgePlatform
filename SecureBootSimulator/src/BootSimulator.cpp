#include "secureboot/BootSimulator.hpp"

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
        const std::string actualHash = verifier_.computeHash(config_.getFirmwarePath());
        const std::string expectedHash = config_.getExpectedSha256();

        if (!verifier_.compareHash(actualHash, expectedHash))
        {
            failureReason_ = "Firmware signature mismatch.";
            return false;
        }
        return true;
    }

    void BootSimulator::run()
    {
        std::cout << "[SecureBoot] Starting boot process...\n"
                  << std::endl;

        if (!verifyFirmware())
        {
            std::cerr << "[SecureBoot] Verification failed: " << failureReason_ << std::endl;
            success_ = false;
            return;
        }
        for (auto &stage : stages_)
        {
            std::cout << "[SecureBoot] ➤ Executing stage: " << stage.getName() << std::endl;
            stage.simulate();

            if (!stage.wasSuccessful())
            {
                failureReason_ = "Stage '" + stage.getName() + "' failed with error code: " + std::to_string(stage.getErrorCode().value());
                std::cerr << "[SecureBoot] " << failureReason_ << std::endl;
                success_ = false;
                return;
            }

            std::cout << "[SecureBoot] Stage '" << stage.getName() << "' completed in " << stage.getDurationMs().value() << " ms\n"
                      << std::endl;
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