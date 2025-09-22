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

            const auto &logs = db_->getLogs();
            if (logs.empty())
            {
                std::cout << "No logs available.\n";
                return;
            }

            std::cout << "Logged Sensor Data:\n";
            std::cout << "---------------------------------------------\n";
            std::cout << std::left
                      << std::setw(12) << "Time(ms)"
                      << std::setw(12) << "Sensor"
                      << std::setw(10) << "Value"
                      << "Faults\n";
            std::cout << "---------------------------------------------\n";

            for (const auto &entry : logs)
            {
                std::cout << std::left
                          << std::setw(12) << entry.timestampMs
                          << std::setw(12) << entry.sensorId
                          << std::setw(10) << std::fixed << std::setprecision(2) << entry.value;

                if (entry.faults.empty())
                {
                    std::cout << "-";
                }
                else
                {
                    for (size_t i = 0; i < entry.faults.size(); ++i)
                    {
                        std::cout << entry.faults[i];
                        if (i != entry.faults.size() - 1)
                            std::cout << ",";
                    }
                }
                std::cout << "\n";
            }

            std::cout << "---------------------------------------------\n";
            std::cout << "Total: " << logs.size() << " entries.\n";
        }

    private:
        MiniDB *db_;
    };
}
