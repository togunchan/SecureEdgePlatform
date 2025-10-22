#pragma once

#include <cppminidb/SensorLogRow.hpp>

namespace channel
{
    class IGatewayChannel
    {
    public:
        virtual ~IGatewayChannel() = default;

        virtual void publish(const cppminidb::SensorLogRow &row) const = 0;
    };
}