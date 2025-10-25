#pragma once

#include <IGatewayChannel.hpp>
#include <edgeagent/EdgeAgent.hpp>
#include <memory>

namespace channel
{
    class AgentChannel : public IGatewayChannel
    {
    public:
        explicit AgentChannel(edgeagent::EdgeAgent *agent);

        void publish(const cppminidb::SensorLogRow &row) const override;

    private:
        edgeagent::EdgeAgent *agent_;
    };
} // namespace channel
