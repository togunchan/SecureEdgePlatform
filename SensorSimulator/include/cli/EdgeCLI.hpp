#pragma once

#include <memory>
#include "../sensors/SimpleSensor.hpp"

class EdgeCLI
{
public:
    void run(); // starts the interactive loop
private:
    void printMenu() const;
    void handleInput(char cmd);
    void generateAndPrintSample();

    std::unique_ptr<sensor::SimpleSensor> sensor_;
    sensor::SensorSpec spec_;
    uint64_t current_time_ms_ = 0;
};
