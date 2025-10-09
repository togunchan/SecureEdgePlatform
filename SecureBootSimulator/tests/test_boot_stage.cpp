#include <catch2/catch_all.hpp>
#include <secureboot/BootStage.hpp>
#include <stdexcept>

using namespace secureboot;

TEST_CASE("BootStage succeeds when simulation handler runs without error")
{
    BootStage stage("Init", 1, []() {});

    stage.simulate();

    REQUIRE(stage.wasSuccessful());
    REQUIRE_FALSE(stage.getErrorCode().has_value());
    REQUIRE(stage.getDurationMs().has_value());
}

TEST_CASE("BootStage reports failure when no simulation handler is provided")
{
    int failCount = 0;
    BootStage stage("Init", 1, nullptr, [&]() { ++failCount; });

    stage.simulate();

    REQUIRE_FALSE(stage.wasSuccessful());
    REQUIRE(stage.getErrorCode().has_value());
    CHECK(stage.getErrorCode().value() == BootStage::kMissingHandlerErrorCode);
    CHECK(failCount == 1);
}

TEST_CASE("BootStage propagates StageFailure error codes")
{
    int failCount = 0;
    BootStage stage("Verify", 1,
                    [&]()
                    { throw StageFailure(42, "verification failed"); },
                    [&]() { ++failCount; });

    stage.simulate();

    REQUIRE_FALSE(stage.wasSuccessful());
    REQUIRE(stage.getErrorCode().has_value());
    CHECK(stage.getErrorCode().value() == 42);
    CHECK(failCount == 1);
}

TEST_CASE("BootStage maps std::exception to default error code")
{
    BootStage stage("Failing", 1, []()
                    { throw std::runtime_error("boom"); });

    stage.simulate();

    REQUIRE_FALSE(stage.wasSuccessful());
    REQUIRE(stage.getErrorCode().has_value());
    CHECK(stage.getErrorCode().value() == BootStage::kUnhandledExceptionErrorCode);
}

TEST_CASE("BootStage resets state between runs")
{
    bool firstRun = true;
    BootStage stage("Flaky", 1, [&]()
                    {
                        if (firstRun)
                        {
                            firstRun = false;
                            throw StageFailure(7, "transient");
                        }
                    });

    stage.simulate();
    REQUIRE_FALSE(stage.wasSuccessful());
    REQUIRE(stage.getErrorCode().has_value());
    CHECK(stage.getErrorCode().value() == 7);

    stage.simulate();
    REQUIRE(stage.wasSuccessful());
    REQUIRE_FALSE(stage.getErrorCode().has_value());
}
