#include <AgentChannel.hpp>
#include <iostream>

namespace channel
{
        AgentChannel::AgentChannel(edgeagent::EdgeAgent *agent) : agent_(agent)
        {
                if (!agent_)
                {
                        std::cerr << "[AgentChannel] Warning: initialized with null EdgeAgent pointer.\n";
                }
        }

        void AgentChannel::publish(const cppminidb::SensorLogRow &row) const
        {
                if (!agent_)
                {
                        std::cerr << "[AgentChannel] No valid EdgeAgent instance. Skipping publish.\n";
                        return;
                }
                std::cout << "[AgentChannel] Sending SensorLogRow to receive(const cppminidb::SensorLogRow &row) function of EdgeAgent object created in EdgeGateway." << std::endl;
                agent_->receive(row);
        }
} // namespace channel
