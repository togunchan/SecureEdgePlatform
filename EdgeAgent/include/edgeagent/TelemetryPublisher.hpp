#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "cppminidb/SensorLogRow.hpp"

namespace edgeagent
{
    class TelemetryPublisher
    {
    public:
        nlohmann::json toJson(const std::vector<cppminidb::SensorLogRow> &rows);

        void publishToConsole(const std::vector<cppminidb::SensorLogRow> &rows);

        void publishToFile(const std::vector<cppminidb::SensorLogRow> &rows,
                           const std::string &filename);

        void publishToMqtt(const std::vector<cppminidb::SensorLogRow> &rows,
                           const std::string &topic);

        void publishToRest(const std::vector<cppminidb::SensorLogRow> &rows,
                           const std::string &url);
    };

} // namespace edgeagent
