/*
 ==============================================================================
 EdgeCLI - Basic Menu-Driven CLI for Fault Injection Testing
 ==============================================================================

 This version of EdgeCLI provides a simple text-menu interface to interact
 with a simulated temperature sensor. It is intended primarily for:

   - Quick manual testing of fault injection logic (spike, dropout, stuck)
   - Educational/demo purposes with minimal CLI complexity
   - Integration into unit tests or interactive sessions

 Note:
 This version supports only a single sensor with hardcoded parameters.
 For more flexible, scriptable, and multi-sensor support,
 see `SensorSimulator/src/cli/EdgeShell.cpp`.

 ==============================================================================
*/

#include "../include/cli/EdgeCLI.hpp"
#include <iostream>
#include <limits>

using namespace sensor;

int main()
{
    std::cout << "Welcome to EdgeCLI - Sensor Fault Injector\n";
    EdgeCLI cli;
    cli.run();
    std::cout << "Exiting EdgeCLI.\n";
    return 0;
}

void EdgeCLI::run()
{
    spec_.id = "TEMP-CLI";
    spec_.type = "TEMP";
    spec_.rate_hz = 1;
    spec_.base = "sine";
    spec_.base_level = 25.0;
    spec_.sine_amp = 2.0;
    spec_.sine_freq_hz = 0.1;
    spec_.noise.gaussian_sigma = 0.1;

    sensor_ = std::make_unique<SimpleSensor>(spec_);
    sensor_->reset(42);

    char cmd;
    while (true)
    {
        printMenu();
        std::cin >> cmd;
        if (cmd == 'q')
            break;
        handleInput(cmd);
    }
}

void EdgeCLI::printMenu() const
{
    std::cout << "\n==== EdgeCLI Menu ====\n"
              << "1. Show next sample\n"
              << "2. Inject spike fault\n"
              << "3. Inject dropout fault\n"
              << "4. Inject stuck fault\n"
              << "5. Reset all faults\n"
              << "q. Quit\n"
              << "Enter command: ";
}

void EdgeCLI::handleInput(char cmd)
{
    switch (cmd)
    {
    case '1':
        generateAndPrintSample();
        break;
    case '2':
        spec_.fault.spike_prob = 1.0;
        spec_.fault.spike_mag = 3.0;
        spec_.fault.spike_sigma = 0.0;
        sensor_ = std::make_unique<SimpleSensor>(spec_);
        sensor_->reset(42);
        std::cout << "Spike fault injected.\n";
        break;
    case '3':
        spec_.fault.dropout_prob = 1.0;
        sensor_ = std::make_unique<SimpleSensor>(spec_);
        sensor_->reset(42);
        std::cout << "Dropout fault injected.\n";
        break;
    case '4':
        spec_.fault.stuck_prob = 1.0;
        spec_.fault.stuck_min_ms = 1000;
        spec_.fault.stuck_max_ms = 1000;
        sensor_ = std::make_unique<SimpleSensor>(spec_);
        sensor_->reset(42);
        std::cout << "Stuck fault injected.\n";
        break;
    case '5':
        spec_.fault = FaultSpec{};
        sensor_ = std::make_unique<SimpleSensor>(spec_);
        sensor_->reset(42);
        std::cout << "All faults cleared.\n";
        break;
    default:
        std::cout << "Invalid command.\n";
        break;
    }
}

void EdgeCLI::generateAndPrintSample()
{
    auto sample = sensor_->nextSample(current_time_ms_);
    current_time_ms_ += 1000;

    std::cout << "Sample @ " << current_time_ms_ << " ms → value: " << sample.value
              << ", quality: " << sample.quality << "\n";
}
