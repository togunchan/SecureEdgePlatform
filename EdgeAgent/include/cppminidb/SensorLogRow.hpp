#pragma once

#include <string>
#include <cstdint>

namespace cppminidb
{

    struct SensorLogRow
    {
        uint64_t timestamp_ms;
        std::string sensor_id;
        double value;
        std::string fault_flags;
    };

} // namespace cppminidb