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
              spike_dist_(0.0),
              uniform_dist_(0.0)
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

            // dropout
            double p = std::clamp(spec_.fault.dropout_prob, 0.0, 1.0);
            dropout_dist_ = std::bernoulli_distribution(p);

            // spike
            double spike_prob = std::clamp(spec_.fault.spike_prob, 0.0, 1.0);
            spike_dist_ = std::bernoulli_distribution(spike_prob);

            if (spec_.fault.spike_sigma > 0.0)
            {
                spike_gauss_dist_ = std::normal_distribution<double>(0.0, spec_.fault.spike_sigma);
            }

            // stuck
            double stuck_prob = std::clamp(spec_.fault.stuck_prob, 0.0, 1.0);
            stuck_prob_dist_ = std::bernoulli_distribution(stuck_prob);
            uint64_t lo = std::min(spec_.fault.stuck_min_ms, spec_.fault.stuck_max_ms);
            uint64_t hi = std::max(spec_.fault.stuck_min_ms, spec_.fault.stuck_max_ms);
            stuck_duration_dist_ = std::uniform_int_distribution<int64_t>(lo, hi);

            // Uniform noise setup
            double r = spec_.noise.uniform_range;
            uniform_dist_ = std::uniform_real_distribution<double>(-r, +r);

            // Drift setup
            double drift_offset_ = 0.0;
            drift_per_sample_ = spec_.noise.drift_ppm * spec_.base_level / 1'000'000.0;

            stuck_until_ms_ = -1;
            last_value_ = std::numeric_limits<double>::quiet_NaN();
        }

        // Generates the next sensor sample for the given timestamp
        Sample nextSample(int64_t now_ms) override
        {
            Sample s = initializeSample(now_ms);
            if (applyDropout(s))
                return s;

            double v = generateBaseSignal(now_ms);
            v += generateNoise(now_ms);

            if (applyStuck(s, v, now_ms))
                return s;

            applySpike(s, v);

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

        SensorSpec &getSpec() override
        {
            return spec_;
        }

    private:
        SensorSpec spec_; // Sensor configuration parameters
        uint64_t seq_;    // Sample sequence counter
        std::mt19937_64 rng_;
        std::normal_distribution<double> dist_;
        std::bernoulli_distribution dropout_dist_;
        int64_t stuck_until_ms_; // Fault simulation placeholder (e.g. stuck state)
        std::bernoulli_distribution spike_dist_;
        std::bernoulli_distribution stuck_prob_dist_{0.0};
        std::uniform_int_distribution<int64_t> stuck_duration_dist_{static_cast<int64_t>(0)};
        double last_value_ = std::numeric_limits<double>::quiet_NaN();
        bool was_stuck_prev = false;
        double gaussian_sigma_;
        std::uniform_real_distribution<double> uniform_dist_;
        double drift_per_sample_ = 0.0;
        double drift_offset_ = 0.0;
        std::normal_distribution<double> spike_gauss_dist_;

        Sample initializeSample(int64_t now_ms)
        {
            Sample s{};          // Create a new sample
            s.ts = now_ms;       // Timestamp in milliseconds
            s.seq = ++seq_;      // Increment and assign sequence number
            s.id = spec_.id;     // Sensor ID (e.g. "TEMP-01")
            s.type = spec_.type; // Sensor type (e.g. "TEMP")
            s.quality |= QF_OK;  // Placeholder for quality metric
            return s;
        }

        bool applyDropout(Sample &s)
        {
            const bool dropout = dropout_dist_(rng_);
            if (dropout)
            {
                s.quality |= QF_DROPOUT;
                s.value = std::numeric_limits<double>::quiet_NaN();
                return true;
            }
            return false;
        }

        double generateBaseSignal(int64_t now_ms)
        {
            double v = spec_.base_level;
            if (spec_.base == "sine" && spec_.sine_amp != 0.0 && spec_.sine_freq_hz > 0.0)
            {
                const double t = now_ms / 1000.0;
                v += spec_.sine_amp * std::sin(2.0 * M_PI * spec_.sine_freq_hz * t);
            }
            return v;
        }

        bool applyStuck(Sample &s, double v, int64_t now_ms)
        {
            if (stuck_until_ms_ >= 0 && now_ms < stuck_until_ms_ && spec_.fault.stuck_prob > 0.0)
            {
                s.quality |= QF_STUCK;
                s.value = last_value_;
                was_stuck_prev = true;
                // std::cout << "[STUCK active] now=" << now_ms
                //           << " until=" << stuck_until_ms_
                //           << " value=" << last_value_ << "\n";
                return true;
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
                    // std::cout << "[STUCK start] now=" << now_ms
                    //           << " dur=" << dur
                    //           << " until=" << stuck_until_ms_ << "\n";
                    return true;
                }
            }
            return false;
        }

        void applySpike(Sample &s, double &v)
        {
            // std::cout << "I am in the applySpike function\n";
            if (!spike_dist_(rng_))
                return;

            s.quality |= QF_SPIKE;
            // std::cout << "Checking if spike_sigma > 0.0\n";
            if (spec_.fault.spike_sigma > 0.0)
            {
                // std::cout << "I am applying the spike because spike_sigma > 0.0\n";
                v += spike_gauss_dist_(rng_);
            }
            else if (spec_.fault.spike_mag > 0.0)
            {
                double spike = (2.0 * spec_.fault.spike_mag) * ((rng_() / (double)rng_.max()) - 0.5);
                v += spike;
            }
            // std::cout << "Spike applied: " << v << "\n";
        }

        double generateNoise(int64_t now_ms)
        {
            double noise = 0.0;

            if (gaussian_sigma_ > 0.0)
            {
                noise += dist_(rng_);
            }

            if (spec_.noise.uniform_range > 0.0)
            {
                std::uniform_real_distribution<double> uniform_dist(
                    -spec_.noise.uniform_range, +spec_.noise.uniform_range);
                noise += uniform_dist(rng_);
            }

            if (spec_.noise.drift_ppm > 0.0)
            {
                const double t_sec = now_ms / 1000.0;
                const double drift_saturation_seconds = 300.0;
                double decay = 1.0 / (1.0 + t_sec / drift_saturation_seconds);
                double drift_rate = decay * spec_.noise.drift_ppm * spec_.base_level / 1'000'000.0;
                noise += drift_rate * t_sec;
            }

            return noise;
        }
    };
}; // namespace senso