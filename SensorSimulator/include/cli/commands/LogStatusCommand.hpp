#pragma once

#include "ICommand.hpp"
#include "../EdgeShell.hpp"
#include <iostream>
#include "../../CppMiniDB/include/cppminidb/MiniDB.hpp"
#include <iomanip>

namespace cli
{
    class LogStatusCommand : public ICommand
    {
    public:
        LogStatusCommand(MiniDB *db) : db_(db) {}

        std::string name() const override
        {
            return "logstatus";
        }
        void execute(const std::vector<std::string> &args) override
        {
            if (!db_)
            {
                std::cout << "Database is not initialized.\n";
                return;
            }

            std::optional<std::string> sensorFilter;
            std::optional<uint64_t> fromTs, toTs;
            std::optional<size_t> lastN;

            bool firstFreeArgConsumed = false;

            for (size_t i = 0; i < args.size(); ++i)
            {
                const auto &a = args[i];
                auto eqPos = a.find('=');
                if (eqPos == std::string::npos)
                {
                    if (!firstFreeArgConsumed)
                    {
                        sensorFilter = a;
                        firstFreeArgConsumed = true;
                    }
                    else
                    {
                    }
                    continue;
                }

                const std::string key = a.substr(0, eqPos);
                const std::string val = a.substr(eqPos + 1);

                try
                {
                    if (key == "sensor")
                        sensorFilter = val;
                    else if (key == "from")
                        fromTs = static_cast<uint64_t>(std::stoull(val));
                    else if (key == "to")
                        toTs = static_cast<uint64_t>(std::stoull(val));
                    else if (key == "last")
                        lastN = static_cast<size_t>(std::stoull(val));
                    else
                    {
                        std::cout << "Unknown filter: " << key << "\n";
                    }
                }
                catch (...)
                {
                    std::cout << "Invalid value for " << key << ": " << val << "\n";
                }
            }

            auto logs = db_->getLogsSnapshot();

            auto match = [&](const LogEntry &e)
            {
                if (sensorFilter && e.sensorId != *sensorFilter)
                    return false;
                if (fromTs && e.timestampMs < *fromTs)
                    return false;
                if (toTs && e.timestampMs > *toTs)
                    return false;
                return true;
            };

            std::vector<LogEntry> filtered;
            filtered.reserve(logs.size());
            for (const auto &e : logs)
                if (match(e))
                    filtered.push_back(e);

            if (lastN && filtered.size() > *lastN)
            {
                filtered.erase(filtered.begin(), filtered.end() - static_cast<std::ptrdiff_t>(*lastN));
            }

            if (filtered.empty())
            {
                std::cout << "No logs match the given filters.\n";
                return;
            }

            std::cout << "Logged Sensor Data";
            if (sensorFilter)
                std::cout << " [sensor=" << *sensorFilter << "]";
            if (fromTs)
                std::cout << " [from=" << *fromTs << "]";
            if (toTs)
                std::cout << " [to=" << *toTs << "]";
            if (lastN)
                std::cout << " [last=" << *lastN << "]";
            std::cout << ":\n";

            std::cout << "---------------------------------------------\n";
            std::cout << std::left
                      << std::setw(12) << "Time(ms)"
                      << std::setw(12) << "Sensor"
                      << std::setw(10) << "Value"
                      << "Faults\n";
            std::cout << "---------------------------------------------\n";

            for (const auto &entry : filtered)
            {
                std::cout << std::left
                          << std::setw(12) << entry.timestampMs
                          << std::setw(12) << entry.sensorId
                          << std::setw(10) << std::fixed << std::setprecision(2) << entry.value;

                if (entry.faults.empty())
                {
                    std::cout << "-\n";
                }
                else
                {
                    for (size_t i = 0; i < entry.faults.size(); ++i)
                    {
                        std::cout << entry.faults[i];
                        if (i + 1 < entry.faults.size())
                            std::cout << ",";
                    }
                    std::cout << "\n";
                }
            }

            std::cout << "---------------------------------------------\n";
            std::cout << "Total: " << filtered.size() << " entries.\n";
        }

    private:
        MiniDB *db_;
    };
}
