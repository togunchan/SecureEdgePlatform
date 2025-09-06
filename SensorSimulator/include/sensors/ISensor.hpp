#pragma once
#include <cstdint>
#include <string>
#include "../sensors/Sample.hpp"
#include "../sensors/Spec.hpp"

namespace sensor
{
    // Polymorphic base class for all sensors
    class ISensor
    {
    public:
        virtual ~ISensor() = default;

        // Deterministic re-initialization for reproducible simulations
        virtual void reset(uint64_t seed) = 0;

        // Produce the next sample for the given time (ms or ns). Pure w.r.t. internal state + now.
        virtual Sample nextSample(int64_t now) = 0;

        // Nominal rate in Hertz (for schedulers)
        virtual int rateHz() const = 0;

        // Stable identity and logical type
        virtual std::string id() const = 0;   // e.g., "TEMP-01"
        virtual std::string type() const = 0; // e.g., "TEMP", "PRES", "IMU-AXIS"
        virtual SensorSpec &getSpec() = 0;
    };
} // namespace sensor