#pragma once
#include <cstdint>
#include <string>

namespace sensor
{

    struct Sample
    {
        int64_t ts;       // timestamp (e.g., ns or ms since epoch)
        uint64_t seq;     // per-sensor monotonically increasing counter
        std::string id;   // sensor unique id (e.g., "TEMP-01")
        std::string type; // e.g., "TEMP", "PRES", "IMU-AXIS"
        double value;     // numeric reading
        uint8_t quality;  // bitmask flags (bit0: ok, bit1: clipped, bit2: dropout, ...)
    };

} // namespace sensor