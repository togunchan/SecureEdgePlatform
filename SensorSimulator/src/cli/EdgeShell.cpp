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
#include "../../include/cli/commands/RunPlotCommand.hpp"
#include "../../include/cli/commands/StopPlotCommand.hpp"
#include "../../include/cli/commands/LogStatusCommand.hpp"
#include "../../include/cli/commands/SaveLogCommand.hpp"
#include "../../include/cli/commands/LoadLogCommand.hpp"
#include "../../include/cli/commands/ClearLogCommand.hpp"
#include "../../include/cli/commands/ExportLogCommand.hpp"
#include "../../include/cli/commands/QueryLogCommand.hpp"
#include "../../include/cli/commands/ImportLogCommand.hpp"
#include "../../include/cli/commands/RemoveCommand.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>

using namespace sensor;

static uint64_t global_time = 0;

void EdgeShell::run(Mode mode)
{
    std::cout << "Welcome to EdgeShell - Multi-Sensor Fault Injector\n";
    current_mode_ = mode;
    printHelp();

    addDefaultSensor();

    if (db_)
    {
        db_->setColumns(
            {"timestamp_ms", "sensor_id", "value", "fault_flags"},
            {MiniDB::ColumnType::Int, MiniDB::ColumnType::String,
             MiniDB::ColumnType::Float, MiniDB::ColumnType::String});
        activeScheduler().setDatabase(db_);
    }

    registry_ = std::make_unique<cli::CommandRegistry>();
    if (mode == Mode::Full)
    {
        registry_->registerCommand(std::make_unique<cli::ListCommand>(*this));
        registry_->registerCommand(std::make_unique<cli::StepCommand>(*this));
        registry_->registerCommand(std::make_unique<cli::InjectCommand>(*this));
        registry_->registerCommand(std::make_unique<cli::ResetCommand>(*this));
        registry_->registerCommand(std::make_unique<cli::AddCommand>(*this));
        registry_->registerCommand(std::make_unique<cli::HelpCommand>(*this));
        registry_->registerCommand(std::make_unique<cli::TickCommand>(*this));
        registry_->registerCommand(std::make_unique<cli::PlotCommand>(*this));
        registry_->registerCommand(std::make_unique<cli::StatusCommand>(activeScheduler()));
        registry_->registerCommand(std::make_unique<cli::RunCommand>(activeScheduler(), is_running_, run_thread_, cv_, cv_mutex_));
        registry_->registerCommand(std::make_unique<cli::StopCommand>(*this));
        registry_->registerCommand(std::make_unique<cli::RunPlotCommand>(*this, is_plotting_, plot_thread_));
        registry_->registerCommand(std::make_unique<cli::StopPlotCommand>(is_plotting_, plot_thread_));
        registry_->registerCommand(std::make_unique<cli::LogStatusCommand>(db_));
        registry_->registerCommand(std::make_unique<cli::SaveLogCommand>(db_));
        registry_->registerCommand(std::make_unique<cli::LoadLogCommand>(db_));
        registry_->registerCommand(std::make_unique<cli::ClearLogCommand>(db_));
        registry_->registerCommand(std::make_unique<cli::ExportLogCommand>(db_));
        registry_->registerCommand(std::make_unique<cli::QueryLogCommand>(db_));
        registry_->registerCommand(std::make_unique<cli::ImportLogCommand>(db_));
        registry_->registerCommand(std::make_unique<cli::RemoveCommand>(*this));
    }
    else
    {
        registry_->registerCommand(std::make_unique<cli::AddCommand>(*this));
        registry_->registerCommand(std::make_unique<cli::ListCommand>(*this));
        registry_->registerCommand(std::make_unique<cli::InjectCommand>(*this));
        registry_->registerCommand(std::make_unique<cli::ResetCommand>(*this));
        registry_->registerCommand(std::make_unique<cli::HelpCommand>(*this));
    }

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
    std::cout << "Commands:\n";
    if (current_mode_ == Mode::Restricted)
    {
        std::cout
            << "  add <id>                     - Add new sensor with given ID\n"
            << "  inject <type> <id> [p1 p2]   - Inject fault (spike/stuck/dropout) with optional params\n"
            << "                                 e.g. inject spike TEMP-001 5.0 0.3\n"
            << "  reset <id>                   - Reset sensor\n"
            << "  list                         - List all sensors\n"
            << "  help                         - Show help\n"
            << "  exit                         - Exit program\n";
        return;
    }
    std::cout
        << "  list                         - List all sensors\n"
        << "  step <id>                    - Generate sample from given sensor\n"
        << "  step all                     - Generate samples from all sensors\n"
        << "  add <id>                     - Add new sensor with given ID\n"
        << "  remove <id>                  - Remove an existing sensor by ID\n"
        << "  tick <delta_ms>              - Advance time and sample as needed\n"
        << "  run                          - Start real-time simulation (ticks every 1s)\n"
        << "  stop                         - Stop real-time simulation\n"
        << "  runplot <id>                 - Start real-time plot\n"
        << "  stopplot                     - Stop real-time plot\n"
        << "  plot <id>                    - Plot sensor data\n"
        << "  status <id>                  - Show active faults on given sensor\n"
        << "  logstatus [filters]          - Show logged sensor entries with optional filters\n"
        << "                                 e.g. logstatus TEMP-001 last=5\n"
        << "  savelog                      - Save logs to .tbl file (in ./data folder)\n"
        << "  loadlog                      - Load logs from disk into memory\n"
        << "  clearlog                     - Clear all logs from memory and disk\n"
        << "  exportlog [options]          - Export logs to JSON file\n"
        << "                                 e.g. exportlog filename=logs.json\n"
        << "                                 e.g. exportlog source=disk filename=backup.json\n"
        << "  querylog <conds> [source=..] - Query logs with conditions\n"
        << "                                 e.g. querylog column=value op== value=25.0\n"
        << "                                 e.g. querylog column=sensor_id op== value=TEMP-001 source=disk\n"
        << "  importlog [options]          - Import logs from JSON into memory or disk\n"
        << "                                 e.g. importlog filename=backup.json\n"
        << "                                 e.g. importlog target=disk filename=logs.json\n"
        << "  inject <type> <id> [p1 p2]   - Inject fault (spike/stuck/dropout) with optional params\n"
        << "                                 e.g. inject spike TEMP-001 5.0 0.3\n"
        << "  reset <id>                   - Reset sensor\n"
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
    if (owned_sensors_.find("TEMP-001") != owned_sensors_.end() || activeScheduler().getScheduledSensor("TEMP-001"))
    {
        return;
    }

    SensorSpec spec = makeDefaultTempSpec();
    spec.id = "TEMP-001";
    spec.type = "TEMP";
    spec.base = "sine";
    spec.base_level = 25.0;
    spec.sine_amp = 0.5; // was 1.5
    spec.sine_freq_hz = 1.0 / 60;
    spec.noise.gaussian_sigma = 0.2;
    auto sensor = std::make_unique<SimpleSensor>(spec);
    ISensor *sensorPtr = sensor.get();
    owned_sensors_["TEMP-001"] = std::move(sensor);
    activeScheduler().addScheduledSensor("TEMP-001", sensorPtr, 1000);
}

void EdgeShell::listSensors() const
{
    auto ids = activeScheduler().getSensorIds();
    if (ids.empty())
    {
        std::cout << "No sensors available.\n";
        return;
    }
    for (const auto &id : ids)
    {
        std::cout << id << "\n";
    }
}

void EdgeShell::stepSensor(const std::string &sensorId)
{
    auto scheduled = activeScheduler().getScheduledSensorAs<sensor::SimpleSensor>(sensorId);
    if (!scheduled)
    {
        std::cout << "Sensor not found: " << sensorId << "\n";
        return;
    }
    auto sample = scheduled->nextSample(global_time);
    global_time += 1000;
    std::cout << "Sample @ " << global_time
              << " ms [" << sensorId << "] → value: " << sample.value
              << "\n";
}

void EdgeShell::injectFault(const std::string &faultType, const std::string &sensorId, const std::vector<std::string> &params)
{
    auto *scheduledSensor = activeScheduler().getScheduledSensorAs<sensor::SimpleSensor>(sensorId);
    if (!scheduledSensor)
    {
        std::cout << "Sensor not scheduled: " << sensorId << "\n";
        return;
    }

    if (faultType == "spike")
    {
        double mag = 3.0;
        double sigma = 0.5;

        if (params.size() >= 1)
            mag = std::stod(params[0]);

        if (params.size() >= 2)
            sigma = std::stod(params[1]);

        int64_t now = activeScheduler().getNow();
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

        int64_t now = activeScheduler().getNow();
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

        auto now = activeScheduler().getNow();

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
    auto *sensor = activeScheduler().getScheduledSensorAs<sensor::SimpleSensor>(sensorId);
    if (!sensor)
    {
        std::cout << "Sensor not found: " << sensorId << "\n";
        return;
    }
    sensor->reset(42);
    std::cout << "Sensor reset: " << sensorId << "\n";
}

void EdgeShell::addScheduledSensor(const std::string &sensorId, uint64_t period_ms)
{
    if (owned_sensors_.find(sensorId) != owned_sensors_.end() || activeScheduler().getScheduledSensor(sensorId))
    {
        std::cout << "Sensor already exists: " << sensorId << "\n";
        return;
    }

    std::string upperId = sensorId;
    std::transform(upperId.begin(), upperId.end(), upperId.begin(), ::toupper);

    SensorSpec spec;
    if (upperId.starts_with("TEMP"))
    {
        spec = makeDefaultTempSpec();
    }
    else if (upperId.starts_with("PRES"))
    {
        spec = makeDefaultPressureSpec();
    }
    else
    {
        printHelp();
        std::cout << "Only TEMP and PRES commands are acceptable at the moment...\n";

        return;
    }
    spec.id = sensorId;

    auto sensor = std::make_unique<SimpleSensor>(spec);
    ISensor *sensorPtr = sensor.get();
    owned_sensors_[sensorId] = std::move(sensor);
    activeScheduler().addScheduledSensor(sensorId, sensorPtr, period_ms);
    std::cout << "Sensor added: " << sensorId << "\n";
}

void EdgeShell::stepAllSensors()
{
    auto ids = activeScheduler().getSensorIds();
    if (ids.empty())
    {
        std::cout << "No sensors available.\n";
        return;
    }

    for (const auto &id : ids)
    {
        auto *sensor = activeScheduler().getScheduledSensorAs<sensor::SimpleSensor>(id);
        if (!sensor)
            continue;
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
        activeScheduler().tick(delta_ms);
    }
}

void EdgeShell::plotSensorData(const std::string &sensorId) const
{
    auto sensor = activeScheduler().getScheduledSensorAs<sensor::SimpleSensor>(sensorId);
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

void EdgeShell::setDatabase(MiniDB *db) { db_ = db; }

const std::unordered_map<std::string, std::unique_ptr<ISensor>> &EdgeShell::getSensors() const
{
    return owned_sensors_;
}

bool EdgeShell::removeSensor(const std::string &id)
{
    activeScheduler().removeScheduledSensor(id);

    auto it = owned_sensors_.find(id);
    if (it != owned_sensors_.end())
    {
        owned_sensors_.erase(it);
        return true;
    }
    return false;
}

void EdgeShell::stop()
{
    if (!is_running_)
    {
        std::cout << "Simulation is not running.\n";
        return;
    }

    {
        std::lock_guard<std::mutex> lock(cv_mutex_);
        is_running_ = false;
    }
    cv_.notify_all();

    if (run_thread_.joinable())
        run_thread_.join();

    std::cout << "Stopped real-time simulation.\n";
}

void EdgeShell::setScheduler(sensor::SensorScheduler *externalScheduler)
{
    external_scheduler_ = externalScheduler;
}
