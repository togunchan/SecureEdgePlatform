#include <catch2/catch_test_macros.hpp>
#include <EdgeGateway.hpp>
#include <IGatewayChannel.hpp>
#include <cppminidb/SensorLogRow.hpp>
#include <memory>
#include <vector>
#include <string>

using namespace gateway;
using namespace cppminidb;

struct DummyChannel : public channel::IGatewayChannel
{
    mutable std::vector<SensorLogRow> received; // mutable so it can be updated inside the const publish override
    void publish(const SensorLogRow &row) const override
    {
        received.push_back(row);
    }
};

TEST_CASE("EdgeGateway publishes sample to all channels", "[edgegateway]")
{
    EdgeGateway gateway;

    std::vector<std::unique_ptr<channel::IGatewayChannel>> testChannels;
    std::unique_ptr dummy1 = std::make_unique<DummyChannel>();
    std::unique_ptr dummy2 = std::make_unique<DummyChannel>();

    DummyChannel *d1 = dummy1.get();
    DummyChannel *d2 = dummy2.get();

    SensorLogRow testRow{1234567890, "sensor-test", 42.0, {"spike"}};

    gateway.setChannelsForTest(std::move(dummy1));
    gateway.setChannelsForTest(std::move(dummy2));
    gateway.setSampleCallbackForTest();

    gateway.injectTestSample(testRow);

    REQUIRE(d1->received[0].sensor_id == "sensor-test");
    REQUIRE(d2->received[0].sensor_id == "sensor-test");
    REQUIRE(d1->received[0].value == 42.0);
    REQUIRE(d2->received[0].value == 42.0);
    REQUIRE(d1->received[0].fault_flags == std::vector<std::string>{"spike"});
    REQUIRE(d2->received[0].fault_flags == std::vector<std::string>{"spike"});
}