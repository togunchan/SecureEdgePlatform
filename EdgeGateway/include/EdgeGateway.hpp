#pragma once
#include <IGatewayChannel.hpp>
#include <scheduler/SensorScheduler.hpp>

namespace gateway
{
    class EdgeGateway
    {
    public:
        void start();
        void setChannelsForTest(std::unique_ptr<channel::IGatewayChannel> ch);
        void setSampleCallbackForTest();
        void injectTestSample(const cppminidb::SensorLogRow &row);

    private:
        std::vector<std::unique_ptr<channel::IGatewayChannel>> channels_;
        sensor::SensorScheduler scheduler_;
    };
} // namespace gateway