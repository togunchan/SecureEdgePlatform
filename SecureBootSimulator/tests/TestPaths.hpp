#pragma once

#include <filesystem>

namespace test_support
{
    inline const std::filesystem::path &dataDirectory()
    {
#ifdef DATA_DIR_PATH
        static const std::filesystem::path path{DATA_DIR_PATH};
#else
        static const std::filesystem::path path =
            std::filesystem::path(__FILE__).parent_path() / "data";
#endif
        return path;
    }
} // namespace test_support
