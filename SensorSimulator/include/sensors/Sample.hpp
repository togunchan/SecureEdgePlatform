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

    constexpr uint8_t QF_OK = 0x00;      // no issues
    constexpr uint8_t QF_DROPOUT = 0x01; // missing sample
    constexpr uint8_t QF_SPIKE = 0x02;   // outlier
    constexpr uint8_t QF_STUCK = 0x04;   // sensor stuck/frozen
    constexpr uint8_t QF_NOISY = 0x08;   // excessive noise detected

} // namespace sensor