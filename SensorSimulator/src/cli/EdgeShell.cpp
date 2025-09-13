#include "../../include/cli/EdgeShell.hpp"
#include "../../include/cli/commands/ListCommand.hpp"
#include "../../include/cli/commands/StepCommand.hpp"
#include "../../include/cli/commands/HelpCommand.hpp"
#include "../../include/cli/commands/InjectCommand.hpp"
#include "../../include/cli/commands/ResetCommand.hpp"
#include "../../include/cli/commands/AddCommand.hpp"
#include "../../include/cli/commands/TickCommand.hpp"

#include <iostream>
#include <sstream>

using namespace sensor;

static uint64_t global_time = 0;

int main()
{
    sensor::EdgeShell shell;
    shell.run();
    return 0;
}

void EdgeShell::run()
{
    std::cout << "Welcome to EdgeShell - Multi-Sensor Fault Injector\n";
    printHelp();

    addDefaultSensor();

    registry_ = std::make_unique<cli::CommandRegistry>();
    registry_->registerCommand(std::make_unique<cli::ListCommand>(*this));
    registry_->registerCommand(std::make_unique<cli::StepCommand>(*this));
    registry_->registerCommand(std::make_unique<cli::InjectCommand>(*this));
    registry_->registerCommand(std::make_unique<cli::ResetCommand>(*this));
    registry_->registerCommand(std::make_unique<cli::AddCommand>(*this));
    registry_->registerCommand(std::make_unique<cli::HelpCommand>(*this));
    registry_->registerCommand(std::make_unique<cli::TickCommand>(*this));

    std::string line;
    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, line);
        if (line == "exit")
            break;
        handleCommand(line);
    }
}

void EdgeShell::printHelp() const
{
    std::cout << "Commands:\n"
              << "  list                         - List all sensors\n"
              << "  step <id>                    - Generate sample from given sensor\n"
              << "  step all                     - Generate samples from all sensors\n"
              << "  inject <type> <id> [p1 p2]   - Inject fault (spike/stuck/dropout) with optional params\n"
              << "                                 e.g. inject spike TEMP-001 5.0 0.3\n"
              << "  reset <id>                   - Reset sensor\n"
              << "  add <id>                     - Add new sensor with given ID\n"
              << "  tick <delta_ms>              - Advance time and sample as needed\n"

              << "  help                         - Show help\n"
              << "  exit                         - Exit program\n";
}

void EdgeShell::handleCommand(const std::string &line)
{
    std::istringstream iss(line);
    std::string cmd, arg;
    std::vector<std::string> args;
    iss >> cmd;

    while (iss >> arg)
    {
        args.push_back(arg);
    }

    if (registry_)
    {
        registry_->executeCommand(cmd, args);
    }
    else
    {
        std::cout << "Command not found: " << cmd << "\n";
    }
}

void EdgeShell::addDefaultSensor()
{
    SensorSpec spec = makeDefaultSpec();
    spec.id = "TEMP-001";
    spec.type = "TEMP";
    spec.base = "sine";
    spec.base_level = 25.0;
    spec.sine_amp = 1.5;
    spec.sine_freq_hz = 1.0 / 60;
    spec.noise.gaussian_sigma = 0.2;
    sensors_["TEMP-001"] = std::make_unique<SimpleTempSensor>(spec);
    auto sensor = std::make_unique<SimpleTempSensor>(spec);
    scheduler_.addScheduledSensor("TEMP-001", std::move(sensor), 1000);
}

void EdgeShell::listSensors() const
{
    for (const auto &sensor : sensors_)
    {
        std::cout << sensor.first << "\n";
    }
}

void EdgeShell::stepSensor(const std::string &sensorId)
{
    // std::cout << "I am in the stepSensor function\n";
    if (sensors_.find(sensorId) == sensors_.end())
    {
        std::cout << "Sensor not found: " << sensorId << "\n";
        return;
    }
    auto &sensor = sensors_[sensorId];
    // std::cout << sensor->getSpec().fault.stuck_prob << "\n";
    auto sample = sensor->nextSample(global_time);
    global_time += 1000;
    std::cout << "Sample @ " << global_time << " ms → value: " << sample.value
              << "\n";
}

void EdgeShell::injectFault(const std::string &faultType, const std::string &sensorId, const std::vector<std::string> &params)
{
    if (sensors_.find(sensorId) == sensors_.end())
    {
        std::cout << "Sensor not found: " << sensorId << "\n";
        return;
    }

    auto &sensor = sensors_[sensorId];

    if (faultType == "spike")
    {
        double mag = 3.0;
        double sigma = 0.5;

        if (params.size() >= 1)
            mag = std::stod(params[0]);

        if (params.size() >= 2)
            sigma = std::stod(params[1]);

        sensor->getSpec().fault.spike_prob = 1.0;
        sensor->getSpec().fault.spike_mag = mag;
        sensor->getSpec().fault.spike_sigma = sigma;

        std::cout << "Injected spike fault on " << sensorId
                  << " [mag=" << mag << ", sigma=" << sigma << "]\n";

        sensor->reset(42);
    }
    else if (faultType == "stuck")
    {
        int min_ms = 1000;
        int max_ms = 1000;

        if (params.size() >= 1)
            min_ms = std::stoi(params[0]);
        if (params.size() >= 2)
            max_ms = std::stoi(params[1]);

        sensor->getSpec().fault.stuck_prob = 1.0;
        sensor->getSpec().fault.stuck_min_ms = min_ms;
        sensor->getSpec().fault.stuck_max_ms = max_ms;

        std::cout << "Injected stuck fault on " << sensorId
                  << " [min=" << min_ms << ", max=" << max_ms << "]\n";

        sensor->reset(42);
    }
    else if (faultType == "dropout")
    {
        sensor->getSpec().fault.dropout_prob = 1.0;
        std::cout << "Injected dropout fault on " << sensorId << "\n";
        sensor->reset(42);
    }
    else
    {
        std::cout << "Unknown fault type: " << faultType << "\n";
    }
}

void EdgeShell::resetSensor(const std::string &sensorId)
{
    if (sensors_.find(sensorId) == sensors_.end())
    {
        std::cout << "Sensor not found: " << sensorId << "\n";
        return;
    }

    sensors_[sensorId]->reset(42);
    std::cout << "Sensor reset: " << sensorId << "\n";
}

void EdgeShell::addSensor(const std::string &sensorId)
{
    if (sensors_.find(sensorId) != sensors_.end())
    {
        std::cout << "Sensor already exists: " << sensorId << "\n";
        return;
    }

    SensorSpec spec = makeDefaultSpec();
    spec.id = sensorId;

    auto sensor = std::make_unique<SimpleTempSensor>(spec);
    sensors_[sensorId] = std::make_unique<SimpleTempSensor>(spec);
    std::cout << "Sensor added: " << sensorId << "\n";
    scheduler_.addScheduledSensor(sensorId, std::move(sensor), 1000);
}

void EdgeShell::stepAllSensors()
{
    if (sensors_.empty())
    {
        std::cout << "No sensors available.\n";
        return;
    }

    std::cout << "[Time: " << global_time << " ms]\n";
    for (auto &[id, sensor] : sensors_)
    {
        auto sample = sensor->nextSample(global_time);
        std::cout << " " << id << " → value: " << sample.value << "\n";
    }
    global_time += 1000;
}

void EdgeShell::tickTime(uint64_t delta_ms)
{
    std::cout << "[Advancing time by " << delta_ms << " ms]\n";
    scheduler_.tick(delta_ms);
}
