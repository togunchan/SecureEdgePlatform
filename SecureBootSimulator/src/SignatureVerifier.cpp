#include "secureboot/SignatureVerifier.hpp"
#include "secureboot/sha256.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

namespace secureboot
{
    std::string SignatureVerifier::computeHash(const std::string &filePath, HashMethod method) const
    {
        // read file as binary
        std::ifstream file(filePath, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Failed to open firmware file: " + filePath);
        }

        std::ostringstream buffer;
        buffer << file.rdbuf(); // read all content

        std::string content = buffer.str();

        if (method == HashMethod::SHA256)
        {
            return sha256(content);
        }
        else if (method == HashMethod::CRC32)
        {
            throw std::runtime_error("CRC32 not yet implemented.");
        }
        else
        {
            throw std::runtime_error("Unknown hash method.");
        }
    }

    bool SignatureVerifier::compareHash(const std::string &actual, const std::string &expected) const
    {
        std::string a = actual;
        std::string e = expected;

        std::transform(a.begin(), a.end(), a.begin(), ::tolower);
        std::transform(e.begin(), e.end(), e.begin(), ::tolower);

        return a == e;
    }
} // namespace secureboot