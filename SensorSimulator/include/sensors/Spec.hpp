#pragma once
#include <string>
#include <cstdint>

namespace sensor
{
    struct NoiseSpec
    {
        double gaussian_sigma = 0.0; // std-dev for N(0, sigma)
        double uniform_range = 0.0;  // +/- range; 0 means disables
        double drift_ppm = 0.0;      // optional slow drift (parts-per-million per second)
    };

    struct FaultSpec
    {
        double spike_prob = 0.0;   // probability of a random spike
        double dropout_prob = 0.0; // probabilty of missing sample
        double spike_mag = 0.0;    // if > 0, spike will be a random value in [-mag, mag]. "mag" stands for magnitude
        double spike_sigma = 0.0;  // if > 0, spike will be a random value in [-sigma, sigma]

        // stuck fault
        double stuck_prob = 0.0;
        uint64_t stuck_min_ms = 0;
        uint64_t stuck_max_ms = 0;
    };

    struct SensorSpec
    {
        std::string id;   // e.g. "TEMP-01"
        std::string type; // e.g. "TEMP"
        int rate_hz = 1;  // nominal rate
        NoiseSpec noise;
        FaultSpec fault;

        // base wave: "constant" / "sine" / "step"
        std::string base = "constant";
        double base_level = 0.0;
        double sine_amp = 0.0;
        double sine_freq_hz = 0.0;
    };

    static inline SensorSpec makeDefaultSpec()
    {
        SensorSpec s;
        s.id = "TEMP-01";
        s.type = "TEMP";
        s.rate_hz = 10;

        s.base = "sine";
        s.base_level = 25.0;
        s.sine_amp = 2.0;
        s.sine_freq_hz = 0.5;

        s.noise.gaussian_sigma = 0.0;

        s.fault.dropout_prob = 0.0;
        s.fault.spike_prob = 0.0;
        s.fault.spike_mag = 0.0;
        s.fault.spike_sigma = 0.0;

        s.fault.stuck_prob = 0.0;
        s.fault.stuck_min_ms = 0;
        s.fault.stuck_max_ms = 0;

        return s;
    }
} // namespace sensor