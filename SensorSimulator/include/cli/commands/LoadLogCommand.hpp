#pragma once

#include "ICommand.hpp"
#include "../EdgeShell.hpp"
#include <iostream>
#include "../../CppMiniDB/include/cppminidb/MiniDB.hpp"

namespace cli
{
    class LoadLogCommand : public ICommand
    {
    public:
        LoadLogCommand(MiniDB *db) : db_(db) {}

        std::string name() const override
        {
            return "loadlog";
        }

        void execute(const std::vector<std::string> &args) override
        {
            if (!db_)
            {
                std::cout << "Database not initialized.\n";
                return;
            }

            db_->loadLogsIntoMemory();

            if (db_->getLogs().empty())
            {
                std::cout << "No logs found on disk.\n";
                return;
            }

            std::cout << "Logs loaded from disk into memory. ("
                      << db_->getLogs().size() << " entries)\n";
        }

    private:
        MiniDB *db_;
    };
}