#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace cppminidb
{
    struct SensorLogRow
    {
        uint64_t timestamp_ms;
        std::string sensor_id;
        double value;
        std::vector<std::string> fault_flags;

        nlohmann::json toJSON() const;
    };
} // namespace cppminidb
