#pragma once

#include <string>
#include <cstdint>

namespace secureboot
{
    class BootConfig
    {
    public:
        bool loadFromFile(const std::string &path);

        const std::string &getFirmwarePath() const;
        const std::string &getExpectedSha256() const;
        const std::string &getBootMode() const;
        uint32_t getEntryPoint() const;

    private:
        std::string firmwarePath_;
        std::string expectedSha256_;
        std::string bootMode_;
        std::string entryPointStr_;
    };
} // namespace secureboot