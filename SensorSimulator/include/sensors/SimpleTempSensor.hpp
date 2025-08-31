#pragma once
#include <random>
#include <cmath>
#include "sensors/ISensor.hpp"
#include "sensors/Spec.hpp"
#include <iostream>

namespace sensor
{
    // A simple temperature sensor simulator that supports sine wave generation and Gaussian noise
    class SimpleTempSensor : public ISensor
    {
    public:
        // Constructor: initializes with a given sensor specification and seeds the RNG
        explicit SimpleTempSensor(const SensorSpec &spec)
            : spec_(spec),
              seq_(0),
              rng_(),
              gaussian_sigma_(0.0),
              dist_(0.0, 1.0),
              dropout_dist_(0.0),
              stuck_until_ms_(std::numeric_limits<int64_t>::max()),
              spike_dist_(0.0)
        {
        }
        // Resets the sensor state and reseeds the random number generator
        void reset(uint64_t seed) override
        {
            seq_ = 0;
            rng_.seed(seed);

            // noise
            gaussian_sigma_ = spec_.noise.gaussian_sigma;
            dist_ = std::normal_distribution<double>(0.0, gaussian_sigma_);
            dist_.reset();

            // dropout
            double p = std::clamp(spec_.fault.dropout_prob, 0.0, 1.0);
            dropout_dist_ = std::bernoulli_distribution(p);

            // spike
            double spike_prob = std::clamp(spec_.fault.spike_prob, 0.0, 1.0);
            spike_dist_ = std::bernoulli_distribution(spike_prob);

            // stuck
            double stuck_prob = std::clamp(spec_.fault.stuck_prob, 0.0, 1.0);
            stuck_prob_dist_ = std::bernoulli_distribution(stuck_prob);
            uint64_t lo = std::min(spec_.fault.stuck_min_ms, spec_.fault.stuck_max_ms);
            uint64_t hi = std::max(spec_.fault.stuck_min_ms, spec_.fault.stuck_max_ms);
            stuck_duration_dist_ = std::uniform_int_distribution<int64_t>(lo, hi);

            stuck_until_ms_ = -1;
            last_value_ = std::numeric_limits<double>::quiet_NaN();
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

            // dropout generation
            const bool dropout = dropout_dist_(rng_);
            if (dropout)
            {
                s.quality |= QF_DROPOUT;
                s.value = std::numeric_limits<double>::quiet_NaN();
                return s;
            }

            // Base signal generation: constant or sine wave
            double v = spec_.base_level; // Start with base level
            if (spec_.base == "sine" && spec_.sine_amp != 0.0 && spec_.sine_freq_hz > 0.0)
            {
                const double t = now_ms / 1000.0;                                    // Convert time to seconds
                v += spec_.sine_amp * std::sin(2.0 * M_PI * spec_.sine_freq_hz * t); // Add sine wave component
            }

            // spike
            if (spike_dist_(rng_) && spec_.fault.spike_mag != 0.0)
            {
                s.quality |= QF_SPIKE;
                v += spec_.fault.spike_mag;
            }

            // Add Gaussian noise if enabled
            if (gaussian_sigma_ > 0.0)
            {
                v += dist_(rng_); // Add random noise to the signal
            }

            // stuck
            if (stuck_until_ms_ >= 0 && now_ms < stuck_until_ms_)
            {
                s.quality |= QF_STUCK;
                s.value = last_value_;
                was_stuck_prev = true;
                std::cout << "[STUCK active] now=" << now_ms
                          << " until=" << stuck_until_ms_
                          << " value=" << last_value_ << "\n";
                return s;
            }

            const bool allow_new_stuck_trial = !was_stuck_prev;
            was_stuck_prev = false;

            if ((stuck_until_ms_ < 0 || now_ms > stuck_until_ms_) && allow_new_stuck_trial && (spec_.fault.stuck_min_ms + spec_.fault.stuck_max_ms > 0) &&
                stuck_prob_dist_(rng_))
            {
                int64_t dur = stuck_duration_dist_(rng_);
                if (dur > 0)
                {
                    stuck_until_ms_ = now_ms + dur;
                    last_value_ = v;
                    s.quality |= QF_STUCK;
                    s.value = last_value_;
                    std::cout << "[STUCK start] now=" << now_ms
                              << " dur=" << dur
                              << " until=" << stuck_until_ms_ << "\n";
                    return s;
                }
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
        SensorSpec spec_; // Sensor configuration parameters
        uint64_t seq_;    // Sample sequence counter
        std::mt19937_64 rng_;
        double gaussian_sigma_;
        std::normal_distribution<double> dist_;
        std::bernoulli_distribution dropout_dist_;
        int64_t stuck_until_ms_; // Fault simulation placeholder (e.g. stuck state)
        std::bernoulli_distribution spike_dist_;
        std::bernoulli_distribution stuck_prob_dist_{0.0};
        std::uniform_int_distribution<int64_t> stuck_duration_dist_{static_cast<int64_t>(0)};
        double last_value_ = std::numeric_limits<double>::quiet_NaN();
        bool was_stuck_prev = false;
    };
} // namespace sensor