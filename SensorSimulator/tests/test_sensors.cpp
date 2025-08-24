#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "sensors/SimpleTempSensor.hpp"
#include <iostream>

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