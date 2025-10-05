#pragma once

#include <string>

namespace secureboot
{
    enum class HashMethod
    {
        SHA256,
        CRC32
    };

    class SignatureVerifier
    {
    public:
        std::string computeHash(const std::string &filePath, HashMethod method = HashMethod::SHA256) const;

        bool compareHash(const std::string &actual, const std::string &expected) const;
    };
} // namespace secureboot