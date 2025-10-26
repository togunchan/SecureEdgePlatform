#include <IGatewayChannel.hpp>

namespace channel
{
    class ConsoleChannel : public IGatewayChannel
    {
    public:
        void publish(const cppminidb::SensorLogRow &row) const override;
    };
} // namespace channel