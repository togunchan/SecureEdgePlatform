#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "sensors/SimpleTempSensor.hpp"
#include <iostream>
#include <cmath>

using namespace sensor;

TEST_CASE("SimpleTempSensor determinism with same seed", "[sensor][determinism]")
{
    SensorSpec spec;
    spec.id = "TEMP-01";
    spec.type = "TEMP";
    spec.rate_hz = 10;
    spec.base = "sine";
    spec.base_level = 25.0;
    spec.sine_amp = 2.0;
    spec.sine_freq_hz = 0.5;
    spec.noise.gaussian_sigma = 0.1;

    SimpleTempSensor a(spec), b(spec);
    a.reset(123);
    b.reset(123);

    auto s1 = a.nextSample(1'000);
    auto s2 = b.nextSample(1'000);
    REQUIRE(s1.value == Catch::Approx(s2.value).epsilon(1e-12));
    std::cout << "s1.value: " << s1.value << std::endl;
    std::cout << "s2.value: " << s2.value << std::endl;
    REQUIRE(s1.seq == s2.seq);
    REQUIRE(s1.id == s2.id);
    REQUIRE(s1.type == s2.type);
}

TEST_CASE("SimpleTempSensor exposes rate and identity", "[sensor][api]")
{
    SensorSpec spec;
    spec.id = "TEMP-01";
    spec.type = "TEMP";
    spec.rate_hz = 5;
    SimpleTempSensor s(spec);
    REQUIRE(s.rateHz() == 5);
    REQUIRE(s.id() == "TEMP-01");
    REQUIRE(s.type() == "TEMP");
}

TEST_CASE("SimpleTempSensor: 100% dropout yields NaN and quality flag", "[sensor][fault][dropout]")
{
    SensorSpec spec;
    spec.id = "TEMP-01";
    spec.type = "TEMP";
    spec.rate_hz = 10;
    spec.base = "constant";
    spec.base_level = 25.0;
    spec.noise.gaussian_sigma = 0.0;
    spec.fault.dropout_prob = 1.0;

    SimpleTempSensor s(spec);
    s.reset(42);

    for (int i = 0; i < 5; ++i)
    {
        auto smp = s.nextSample(1000 + i * 100);
        REQUIRE((smp.quality & QF_DROPOUT) != 0);
        REQUIRE(std::isnan(smp.value));
    }
}

TEST_CASE("SimpleTempSensor: 0% dropout produces normal samples", "[sensor][fault][dropout]")
{
    SensorSpec spec;
    spec.id = "TEMP-02";
    spec.type = "TEMP";
    spec.rate_hz = 10;
    spec.base = "constant";
    spec.base_level = 25.0;
    spec.noise.gaussian_sigma = 0.0;
    spec.fault.dropout_prob = 0.0;

    SimpleTempSensor s(spec);
    s.reset(7);

    for (int i = 0; i < 5; ++i)
    {
        auto smp = s.nextSample(2000 + i * 100);
        REQUIRE((smp.quality & QF_DROPOUT) == 0);
        REQUIRE_FALSE(std::isnan(smp.value));
    }
}

TEST_CASE("Spike always on when prob = 1", "[sensor][fault][spike]")
{
    SensorSpec spec;
    spec.id = "TEMP-01";
    spec.type = "TEMP";
    spec.rate_hz = 1;
    spec.base = "constant";
    spec.base_level = 20.0;
    spec.noise.gaussian_sigma = 0.0;
    spec.fault.dropout_prob = 0.0;
    spec.fault.spike_prob = 1.0;
    spec.fault.spike_mag = 5.0;
    spec.fault.spike_sigma = 0.0;

    SimpleTempSensor s(spec);
    s.reset(42);
    auto smp = s.nextSample(1000);
    REQUIRE((smp.quality & QF_SPIKE) != 0);
    REQUIRE(smp.value >= 15.0); // 20 - 5
    REQUIRE(smp.value <= 25.0); // 20 + 5
}

// Note: Declared as 'static' so that this helper is visible only within this
// translation unit (test_sensors.cpp). This prevents global symbol pollution
// and avoids potential linker conflicts if other test files also define a
// makeDefaultSpec() function.
// static SensorSpec makeDefaultSpec()
// {
//     SensorSpec spec;
//     spec.id = "TEMP-01";
//     spec.type = "TEMP";
//     spec.rate_hz = 10;
//     spec.base = "sine";
//     spec.base_level = 25.0;
//     spec.sine_amp = 2.0;
//     spec.sine_freq_hz = 0.5;
//     spec.noise.gaussian_sigma = 0.0;
//     spec.fault.dropout_prob = 0.0;
//     spec.fault.spike_prob = 0.0;
//     spec.fault.spike_mag = 0.0;
//     return spec;
// }

TEST_CASE("Dropout dominates over spike", "[sensor][fault][dropout][spike]")
{
    auto spec = makeDefaultSpec();
    spec.fault.dropout_prob = 1.0;
    spec.fault.spike_prob = 1.0;
    spec.fault.spike_mag = 5.0;

    SimpleTempSensor s(spec);
    s.reset(123);

    auto sample = s.nextSample(1'000);

    REQUIRE(std::isnan(sample.value));
    REQUIRE((sample.quality & QF_DROPOUT) != 0);
    REQUIRE((sample.quality & QF_SPIKE) == 0);
}

// At t=1000ms with f=0.5Hz, the sine wave term equals sin(pi)=0.
// This means the sine component contributes nothing, so the output
// should be exactly: base_level (+spike, +noise if enabled).
// Using this time point isolates the effect of spike/noise from the sine.
TEST_CASE("Spike applied without noise at t=1000ms (sin term is zero)", "[sensor][fault][spike]")
{
    auto spec = makeDefaultSpec();
    spec.fault.spike_prob = 1.0;
    spec.fault.spike_mag = 3.5;
    spec.fault.spike_sigma = 0.0;
    spec.noise.gaussian_sigma = 0.0;
    spec.fault.dropout_prob = 0.0;

    SimpleTempSensor s(spec);
    s.reset(42);

    auto sample = s.nextSample(1'000);
    double min = 25.0 - 3.5;
    double max = 25.0 + 3.5;
    REQUIRE(sample.value >= min);
    REQUIRE(sample.value <= max);
    REQUIRE((sample.quality & QF_SPIKE) != 0);
}

TEST_CASE("No spike when spike_prob=0, no dropout, no noise", "[sensor][fault][spike]")
{
    auto spec = makeDefaultSpec();
    spec.fault.spike_prob = 0.0;
    spec.fault.dropout_prob = 0.0;
    spec.noise.gaussian_sigma = 0.0;

    SimpleTempSensor s(spec);
    s.reset(7);

    auto sample = s.nextSample(1'000); // base = 25.0
    REQUIRE(sample.value == 25.0);
    REQUIRE((sample.quality & QF_SPIKE) == 0);
    REQUIRE((sample.quality & QF_DROPOUT) == 0);
}

TEST_CASE("Spike always vs never across many samples", "[sensor][fault][spike][prob]")
{
    {
        auto spec = makeDefaultSpec();
        spec.fault.spike_prob = 1.0;
        spec.fault.spike_mag = 1.0;
        spec.fault.spike_sigma = 0.0;
        spec.noise.gaussian_sigma = 0.0;
        spec.fault.dropout_prob = 0.0;

        SimpleTempSensor s(spec);
        s.reset(99);

        for (int i = 0; i < 20; ++i)
        {
            auto sample = s.nextSample(1'000 + i * 100);
            REQUIRE((sample.quality & QF_SPIKE) != 0);
            REQUIRE_FALSE(std::isnan(sample.value));
        }
    }

    {
        auto spec = makeDefaultSpec();
        spec.fault.spike_prob = 0.0;
        spec.noise.gaussian_sigma = 0.0;
        spec.fault.spike_sigma = 0.0;
        spec.fault.dropout_prob = 0.0;

        SimpleTempSensor s(spec);
        s.reset(100);

        for (int i = 0; i < 20; ++i)
        {
            auto sample = s.nextSample(1'000 + i * 100);
            REQUIRE((sample.quality & QF_SPIKE) == 0);
            REQUIRE_FALSE(std::isnan(sample.value));
        }
    }
}

TEST_CASE("Stuck freezes value within windows", "[sensor][fault][stuck]")
{
    SensorSpec spec = makeDefaultSpec();
    spec.base = "constant";
    spec.base_level = 25.0;
    spec.noise.gaussian_sigma = 0.0;
    spec.fault.dropout_prob = 0.0;
    spec.fault.spike_prob = 0.0;

    spec.fault.stuck_prob = 1.0;
    spec.fault.stuck_min_ms = 3000;
    spec.fault.stuck_max_ms = 3000;

    SimpleTempSensor s(spec);
    s.reset(123);

    auto a = s.nextSample(500);
    auto b = s.nextSample(1000);
    auto c = s.nextSample(2500);
    auto d = s.nextSample(4000);

    REQUIRE(std::isfinite(a.value));
    REQUIRE(b.quality & QF_STUCK);
    REQUIRE(c.quality & QF_STUCK);
    REQUIRE(b.value == a.value);
    REQUIRE(c.value == a.value);
    REQUIRE_FALSE(d.quality & QF_STUCK);
}

TEST_CASE("Dropout has precedence over stuck/spike/noise", "[sensor][fault][priority]")
{
    SensorSpec spec = makeDefaultSpec();
    spec.base = "constant";
    spec.base_level = 10.0;
    spec.noise.gaussian_sigma = 1.0;
    spec.fault.dropout_prob = 1.0;
    spec.fault.spike_prob = 1.0;
    spec.fault.spike_mag = 5.0;
    spec.fault.stuck_prob = 1.0;
    spec.fault.stuck_min_ms = 1000;
    spec.fault.stuck_max_ms = 1000;

    SimpleTempSensor s(spec);
    s.reset(42);

    auto x = s.nextSample(1000);
    REQUIRE(x.quality & QF_DROPOUT);
    REQUIRE(std::isnan(x.value));
}

TEST_CASE("SimpleTempSensor: decaying positive drift increases value over time", "[sensor][drift]")
{
    SensorSpec spec = makeDefaultSpec();
    spec.base = "constant";
    spec.base_level = 25.0;
    spec.noise.gaussian_sigma = 0.0;
    spec.noise.uniform_range = 0.0;
    spec.noise.drift_ppm = 100'000.0; // intentionally large for visible change
    spec.fault.dropout_prob = 0.0;
    spec.fault.spike_prob = 0.0;
    spec.fault.stuck_prob = 0.0;

    SimpleTempSensor sensor(spec);
    sensor.reset(42);

    std::vector<double> values;
    for (int i = 0; i <= 10; ++i)
    {
        auto smp = sensor.nextSample(i * 1000);
        REQUIRE_FALSE(std::isnan(smp.value));
        values.push_back(smp.value);
    }

    // Values should be strictly increasing
    for (size_t i = 1; i < values.size(); ++i)
    {
        REQUIRE(values[i] > values[i - 1]);
    }

    // Drift effect should decay. Differences get smaller
    for (size_t i = 2; i < values.size(); ++i)
    {
        double diff1 = values[i - 1] - values[i - 2];
        double diff2 = values[i] - values[i - 1];
        REQUIRE(diff2 < diff1); // drift impact is weakening
    }
}

TEST_CASE("SimpleTempSensor: uniform noise affects sample value", "[sensor][noise][uniform]")
{
    SensorSpec spec = makeDefaultSpec();
    spec.base = "constant";
    spec.base_level = 42.0;
    spec.noise.gaussian_sigma = 0.0;
    spec.noise.uniform_range = 1.0;
    spec.noise.drift_ppm = 0.0;
    spec.fault.dropout_prob = 0.0;
    spec.fault.stuck_prob = 0.0;
    spec.fault.spike_prob = 0.0;

    SimpleTempSensor sensor(spec);
    sensor.reset(123);

    for (int i = 0; i < 20; ++i)
    {
        Sample smp = sensor.nextSample(i * 1000);
        REQUIRE(smp.value >= 41.0);
        REQUIRE(smp.value <= 43.0);
    }
}

TEST_CASE("SimpleTempSensor: gaussian noise is within expected range", "[sensor][noise][gaussian]")
{
    SensorSpec spec = makeDefaultSpec();
    spec.base = "constant";
    spec.base_level = 50.0;
    spec.noise.gaussian_sigma = 1.0;
    spec.noise.uniform_range = 0.0;
    spec.noise.drift_ppm = 0.0;
    spec.fault.dropout_prob = 0.0;
    spec.fault.stuck_prob = 0.0;
    spec.fault.spike_prob = 0.0;

    SimpleTempSensor sensor(spec);
    sensor.reset(123);

    for (int i = 0; i < 100; ++i)
    {
        Sample smp = sensor.nextSample(i * 1000);
        // Not strict: 6σ covers 99.999%
        REQUIRE(smp.value >= 44.0); // 50 - 6
        REQUIRE(smp.value <= 56.0); // 50 + 6
    }
}

TEST_CASE("SimpleTempSensor: combined gaussian, uniform, and drift noise", "[sensor][noise][combined]")
{
    SensorSpec spec = makeDefaultSpec();
    spec.base = "constant";
    spec.base_level = 100.0;

    // Activate all noise types
    spec.noise.gaussian_sigma = 1.0;
    spec.noise.uniform_range = 1.0;
    spec.noise.drift_ppm = 50'000.0;
    spec.fault.dropout_prob = 0.0;
    spec.fault.stuck_prob = 0.0;
    spec.fault.spike_prob = 0.0;

    SimpleTempSensor sensor(spec);
    sensor.reset(321);

    std::vector<double> values;
    for (int i = 0; i < 30; ++i)
    {
        auto smp = sensor.nextSample(i * 1000);
        REQUIRE_FALSE(std::isnan(smp.value));
        values.push_back(smp.value);
    }

    const double base = spec.base_level;
    const double max_uniform = spec.noise.uniform_range;
    const double max_gaussian = 6.0 * spec.noise.gaussian_sigma;
    const double margin = 3.0;
    const double drift_saturation_seconds = 300.0;

    for (int i = 0; i < 20; ++i)
    {
        int64_t t_ms = i * 1000;
        double t_sec = t_ms / 1000.0;

        Sample smp = sensor.nextSample(t_ms);
        double v = smp.value;

        double decay = 1.0 / (1.0 + t_sec / drift_saturation_seconds);
        double drift_rate = decay * spec.noise.drift_ppm * base / 1'000'000.0;
        double drift = drift_rate * t_sec;

        double upper_bound = base + drift + max_uniform + max_gaussian + margin;
        double lower_bound = base + drift - (max_uniform + max_gaussian + margin);

        INFO("t=" << t_sec << "s → v=" << v << ", allowed: [" << lower_bound << ", " << upper_bound << "]");
        REQUIRE(v >= lower_bound);
        REQUIRE(v <= upper_bound);
    }

    // There should be some upward trend due to drift
    bool has_increase = false;
    for (size_t i = 1; i < values.size(); ++i)
    {
        if (values[i] > values[i - 1])
        {
            has_increase = true;
            break;
        }
    }
    REQUIRE(has_increase);
}

TEST_CASE("SimpleTempSensor: dropout disables all other faults and noise", "[sensor][fault][priority]")
{
    SensorSpec spec = makeDefaultSpec();
    spec.base = "constant";
    spec.base_level = 42.0;

    spec.noise.gaussian_sigma = 1.0;
    spec.noise.uniform_range = 1.0;
    spec.noise.drift_ppm = 10'000.0;

    spec.fault.dropout_prob = 1.0; // Always dropout
    spec.fault.stuck_prob = 1.0;
    spec.fault.spike_prob = 1.0;

    SimpleTempSensor s(spec);
    s.reset(42);

    for (int i = 0; i < 10; ++i)
    {
        auto smp = s.nextSample(i * 1000);

        // Should be dropout
        REQUIRE(std::isnan(smp.value));
        REQUIRE((smp.quality & QF_DROPOUT) != 0);

        // All other faults should be skipped
        REQUIRE((smp.quality & QF_SPIKE) == 0);
        REQUIRE((smp.quality & QF_STUCK) == 0);
    }
}