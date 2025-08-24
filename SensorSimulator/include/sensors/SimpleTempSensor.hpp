#pragma once
#include <random>
#include <cmath>
#include "sensors/ISensor.hpp"
#include "sensors/Spec.hpp"

namespace sensor
{
    // A simple temperature sensor simulator that supports sine wave generation and Gaussian noise
    class SimpleTempSensor : public ISensor
    {
    public:
        // Constructor: initializes with a given sensor specification and seeds the RNG
        explicit SimpleTempSensor(const SensorSpec &spec) : spec_(spec), seq_(0)
        {
            rng_.seed(0); // Fixed seed for deterministic noise
        }

        // Resets the sensor state and reseeds the random number generator
        void reset(uint64_t seed) override
        {
            seq_ = 0;             // Reset sample sequence counter
            rng_.seed(seed);      // Reseed RNG for new noise pattern
            stuck_untik_ms_ = -1; // Reset fault state (not yet implemented)
        }

        // Generates the next sensor sample for the given timestamp
        Sample nextSample(int64_t now_ms) override
        {
            Sample s{};          // Create a new sample
            s.ts = now_ms;       // Timestamp in milliseconds
            s.seq = ++seq_;      // Increment and assign sequence number
            s.id = spec_.id;     // Sensor ID (e.g. "TEMP-01")
            s.type = spec_.type; // Sensor type (e.g. "TEMP")
            s.quality = 0;       // Placeholder for quality metric

            // Base signal generation: constant or sine wave
            double v = spec_.base_level; // Start with base level
            if (spec_.base == "sine" && spec_.sine_amp != 0.0 && spec_.sine_freq_hz > 0.0)
            {
                const double t = now_ms / 1000.0;                                    // Convert time to seconds
                v += spec_.sine_amp * std::sin(2.0 * M_PI * spec_.sine_freq_hz * t); // Add sine wave component
            }

            // Add Gaussian noise if enabled
            if (spec_.noise.gaussian_sigma > 0.0)
            {
                std::normal_distribution<double> dist(0.0, spec_.noise.gaussian_sigma); // N(0, sigma)
                v += dist(rng_);                                                        // Add random noise to the signal
            }

            s.value = v; // Final computed sensor value
            return s;    // Return the sample
        }

        // Returns the sampling rate in Hz
        int rateHz() const override
        {
            return spec_.rate_hz;
        }

        // Returns the sensor ID
        std::string id() const override
        {
            return spec_.id;
        }

        // Returns the sensor type
        std::string type() const override
        {
            return spec_.type;
        }

    private:
        SensorSpec spec_;              // Sensor configuration parameters
        uint64_t seq_;                 // Sample sequence counter
        std::mt19937_64 rng_;          // 64-bit Mersenne Twister RNG
        uint64_t stuck_untik_ms_ = -1; // Fault simulation placeholder (e.g. stuck state)
    };
} // namespace sensor