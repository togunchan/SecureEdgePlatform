#pragma once

#include "ICommand.hpp"
#include "../EdgeShell.hpp"
#include <iostream>
#include <string>
#include "../../CppMiniDB/include/cppminidb/MiniDB.hpp"

namespace cli
{
    class SaveLogCommand : public ICommand
    {
    public:
        SaveLogCommand(MiniDB *db) : db_(db) {}

        std::string name() const override
        {
            return "savelog";
        }

        void execute(const std::vector<std::string> &args) override
        {
            try
            {
                db_->save();
                std::cout << "Logs successfully saved..." << "\n ";
            }
            catch (const std::exception &e)
            {
                std::cout << "Error saving logs!: " << e.what() << "\n";
            }
        }

    private:
        MiniDB *db_;
    };
}