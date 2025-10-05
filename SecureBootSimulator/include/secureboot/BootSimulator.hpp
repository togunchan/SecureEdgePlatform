#pragma once

#include <vector>
#include <string>

#include "BootConfig.hpp"
#include "BootStage.hpp"
#include "SignatureVerifier.hpp"

namespace secureboot
{
    class BootSimulator
    {
    public:
        explicit BootSimulator(const BootConfig &config, HashMethod hashMethod = HashMethod::SHA256);

        void addStage(const BootStage &stage);
        void run();

        bool wasSuccessful() const;
        std::string getFailureReason() const;

    private:
        BootConfig config_;
        SignatureVerifier verifier_;
        HashMethod hashMethod_;
        std::vector<BootStage> stages_;

        bool success_ = false;
        std::string failureReason_;

        bool verifyFirmware();
    };
} // namespace secureboot