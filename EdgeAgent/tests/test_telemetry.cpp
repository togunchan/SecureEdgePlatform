#include "edgeagent/TelemetryPublisher.hpp"
#include "cppminidb/SensorLogRow.hpp"

#include <iostream>
#include <vector>
#include <string>

int main()
{
    std::vector<cppminidb::SensorLogRow> logs = {
        {1695979200000, "sensor-001", 36.5, {""}},
        {1695979205000, "sensor-001", 36.9, {"spike"}},
        {1695979210000, "sensor-002", 20.3, {"dropout"}},
        {1695979215000, "sensor-003", 25.0, {""}}};

    edgeagent::TelemetryPublisher publisher;

    std::cout << "=== Console Output ===" << std::endl;
    publisher.publishToConsole(logs);

    const std::string outputFile = "data/test_output.json";
    try
    {
        publisher.publishToFile(logs, outputFile);
        std::cout << "\n JSON file successfully written to: " << outputFile << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "\n Failed to write JSON file: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}