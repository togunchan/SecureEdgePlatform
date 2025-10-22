#pragma once
#include <IGatewayChannel.hpp>
#include <scheduler/SensorScheduler.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace gateway
{
    class EdgeGateway
    {
    public:
        void start();
        void setChannelsForTest(std::unique_ptr<channel::IGatewayChannel> ch);
        void setSampleCallbackForTest();
        void injectTestSample(const cppminidb::SensorLogRow &row);
        void runLoop();

    private:
        std::vector<std::unique_ptr<channel::IGatewayChannel>> channels_;
        sensor::SensorScheduler scheduler_;
        std::unordered_map<std::string, std::unique_ptr<sensor::ISensor>> sensors_;
    };
} // namespace gateway
