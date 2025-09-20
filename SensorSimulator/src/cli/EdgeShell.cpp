#include "../../include/cli/EdgeShell.hpp"
#include "../../include/cli/commands/ListCommand.hpp"
#include "../../include/cli/commands/StepCommand.hpp"
#include "../../include/cli/commands/HelpCommand.hpp"
#include "../../include/cli/commands/InjectCommand.hpp"
#include "../../include/cli/commands/ResetCommand.hpp"
#include "../../include/cli/commands/AddCommand.hpp"
#include "../../include/cli/commands/TickCommand.hpp"
#include "../../include/cli/commands/PlotCommand.hpp"
#include "../../include/cli/commands/StatusCommand.hpp"
#include "../../include/cli/commands/RunCommand.hpp"
#include "../../include/cli/commands/StopCommand.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>

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
    registry_->registerCommand(std::make_unique<cli::PlotCommand>(*this));
    registry_->registerCommand(std::make_unique<cli::StatusCommand>(scheduler_));
    registry_->registerCommand(std::make_unique<cli::RunCommand>(scheduler_, is_running_, run_thread_));
    registry_->registerCommand(std::make_unique<cli::StopCommand>(is_running_, run_thread_));

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
              << "  run                          - Start real-time simulation (ticks every 1s)\n"
              << "  stop                         - Stop real-time simulation\n"
              << "  plot <id>                    - Plot sensor data\n"
              << "  status <id>                  - Show active faults on given sensor\n"
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
    spec.sine_amp = 0.0; // was 1.5
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
    auto *scheduledSensor = scheduler_.getScheduledSensor(sensorId);

    if (faultType == "spike")
    {
        double mag = 3.0;
        double sigma = 0.5;

        if (params.size() >= 1)
            mag = std::stod(params[0]);

        if (params.size() >= 2)
            sigma = std::stod(params[1]);

        int64_t now = scheduler_.getNow();
        scheduledSensor->triggerSpikeFault(mag, sigma, now);

        std::cout << "Triggered transient spike on " << sensorId
                  << " [mag=" << mag << ", sigma=" << sigma << "]\n";
    }
    else if (faultType == "stuck")
    {
        int duration_ms = 1000;
        if (params.size() >= 1)
            duration_ms = std::stoi(params[0]);
        std::cout << "Duration MS is ->> " << duration_ms << "\n";

        int64_t now = scheduler_.getNow();
        double current_value = scheduledSensor->getHistory().empty()
                                   ? scheduledSensor->getSpec().base_level
                                   : scheduledSensor->getHistory().back();

        scheduledSensor->triggerStuckFault(duration_ms, now, current_value);

        std::cout << "Triggered transient stuck fault on " << sensorId
                  << " [duration=" << duration_ms << " ms]\n";
    }
    else if (faultType == "dropout")
    {
        int duration_ms = 2000;
        if (!params.empty())
            duration_ms = std::stoi(params[0]);

        auto now = scheduler_.getNow();

        scheduledSensor->triggerDropoutFault(now, duration_ms);

        std::cout << "Injected dropout fault on " << sensorId
                  << " [duration=" << duration_ms << "ms]\n";
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

void EdgeShell::addScheduledSensor(const std::string &sensorId, uint64_t period_ms)
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
    scheduler_.addScheduledSensor(sensorId, std::move(sensor), period_ms);
    std::cout << "Sensor added: " << sensorId << "\n";
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
    for (int i = 0; i < 25; ++i)
    {
        scheduler_.tick(delta_ms);
    }
}

void EdgeShell::plotSensorData(const std::string &sensorId) const
{
    auto sensor = scheduler_.getScheduledSensor(sensorId);
    if (!sensor)
    {
        std::cout << "Sensor not found: " << sensorId << "\n";
        return;
    }

    const auto &history = sensor->getHistory();
    if (history.empty())
    {
        std::cout << "No data to plot for " << sensorId << "\n";
        return;
    }

    // Min/max only from valid (non-NaN) values
    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::lowest();
    for (double val : history)
    {
        if (!std::isnan(val))
        {
            min = std::min(min, val);
            max = std::max(max, val);
        }
    }

    double range = (max - min == 0) ? 1.0 : max - min;
    const int height = 10;
    std::vector<std::string> lines(height, std::string(history.size(), ' '));

    for (size_t i = 0; i < history.size(); ++i)
    {
        double val = history[i];
        if (std::isnan(val))
        {
            std::cout << "[DEBUG] NaN at index " << i << "\n";
            int middle = height / 2;
            lines[middle][i] = 'X';
        }
        else
        {
            int level = static_cast<int>((val - min) / range * (height - 1));
            level = std::clamp(level, 0, height - 1);
            lines[height - 1 - level][i] = '#';
        }
    }

    std::cout << "Plotting " << sensorId << " (last " << history.size() << " samples)\n\n";
    for (int i = 0; i < height; ++i)
    {
        double label = max - (range * i) / (height - 1);
        std::cout << std::fixed << std::setw(6) << std::setprecision(2) << label << " ┤ " << lines[i] << "\n";
    }

    // X-axis line
    std::cout << "       └";
    for (size_t i = 0; i < history.size(); ++i)
        std::cout << "─";
    std::cout << "→ Time\n";

    // Tick marks
    std::cout << "        ";
    for (size_t i = 0; i < history.size(); ++i)
        std::cout << ((i % 10 == 0) ? "|" : " ");
    std::cout << "\n";

    // Time labels
    std::cout << "        ";
    for (size_t i = 0; i < history.size(); ++i)
    {
        if (i % 10 == 0)
        {
            std::ostringstream oss;
            oss << i;
            std::cout << oss.str();
            i += oss.str().length() - 1;
        }
        else
        {
            std::cout << " ";
        }
    }
    std::cout << "\n";
}