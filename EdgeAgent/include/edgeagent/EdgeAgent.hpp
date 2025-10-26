#pragma once

#include <vector>
#include <cppminidb/SensorLogRow.hpp>
#include <edgeagent/TelemetryPublisher.hpp>

namespace edgeagent
{
    class EdgeAgent
    {
    public:
        void receive(const cppminidb::SensorLogRow &row);

        void flushToConsole();

        void flushToFile(const std::string &filename);

    private:
        TelemetryPublisher publisher_;
        std::vector<cppminidb::SensorLogRow> buffer_;
    };
} // namespace edgeagent