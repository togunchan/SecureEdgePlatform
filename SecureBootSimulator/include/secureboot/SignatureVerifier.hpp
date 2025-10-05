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
        SignatureVerifier::SignatureVerifier(HashMethod method)
            : method_(method) {}
        std::string computeHash(const std::string &filePath) const;

        bool compareHash(const std::string &actual, const std::string &expected) const;

    private:
        HashMethod method_;
    };
} // namespace secureboot