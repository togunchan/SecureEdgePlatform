#include "edgeagent/TelemetryPublisher.hpp"
#include <iostream>
#include <fstream>

namespace edgeagent
{
    nlohmann::json TelemetryPublisher::toJson(const std::vector<cppminidb::SensorLogRow> &rows)
    {
        nlohmann::json json_array = nlohmann::json::array();
        for (auto &row : rows)
        {
            nlohmann::json json_row;
            json_row["timestamp_ms"] = row.timestamp_ms;
            json_row["sensor_id"] = row.sensor_id;
            json_row["value"] = row.value;
            json_row["fault_flags"] = row.fault_flags;

            json_array.push_back(json_row);
        }
        return json_array;
    }

    void TelemetryPublisher::publishToConsole(const std::vector<cppminidb::SensorLogRow> &rows)
    {
        nlohmann::json data = toJson(rows);
        std::cout << data.dump(4) << std::endl;
    }

    void TelemetryPublisher::publishToFile(const std::vector<cppminidb::SensorLogRow> &rows, const std::string &filename)
    {
        nlohmann::json data = toJson(rows);
        std::ofstream outFile(filename);

        if (!outFile)
        {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        outFile << data.dump(4);
        outFile.close();
    }

}
