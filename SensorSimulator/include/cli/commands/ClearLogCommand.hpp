#pragma once

#include "ICommand.hpp"
#include <unordered_map>
#include "scheduler/SensorScheduler.hpp"
#include "../../CppMiniDB/include/cppminidb/MiniDB.hpp"

namespace cli
{
    class ClearLogCommand : public ICommand
    {
    public:
        ClearLogCommand(MiniDB *db) : db_(db) {}

        std::string name() const override
        {
            return "clearlog";
        }

        void execute(const std::vector<std::string> &args) override
        {
            if (!db_)
            {
                std::cout << "MiniDB not initialized.\n";
                return;
            }

            db_->clear();       // clear rows + reset file with header
            db_->clearMemory(); // clear logs_ vector

            std::cout << "Logs cleared from memory and disk.\n";
        }

    private:
        MiniDB *db_;
    };
}