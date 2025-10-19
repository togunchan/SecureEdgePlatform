#pragma once
#include <IGatewayChannel.hpp>

namespace gateway
{
    class EdgeGateway
    {
    public:
        void start();

    private:
        std::vector<std::unique_ptr<channel::IGatewayChannel>> channels_;
    };
} // namespace gateway