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
        int stuck_duration_ms = 0; // if > 0, sensor can get "stuck" for this long
        double spike_mag = 0.0;    // if > 0, spike will be a random value in [-mag, mag]. "mag" stands for magnitude
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
} // namespace sensor