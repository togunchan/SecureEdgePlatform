#pragma once

#include "ICommand.hpp"
#include "../EdgeShell.hpp"
#include "../../CppMiniDB/include/cppminidb/MiniDB.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

namespace cli
{
    class ImportLogCommand : public ICommand
    {
    public:
        ImportLogCommand(MiniDB *db) : db_(db) {};

        std::string name() const override
        {
            return "importlog";
        }

        void execute(const std::vector<std::string> &args) override
        {
            std::string filename = "./data/logs.json";
            std::string target = "memory";

            for (const auto &arg : args)
            {
                if (arg.starts_with("filename="))
                {
                    filename = arg.substr(9);
                }
                else if (arg.starts_with("target="))
                {
                    target = arg.substr(7);
                }
            }

            std::ifstream ifs(filename);
            if (!ifs)
            {
                std::cout << "Failed to open file: " << filename << "\n";
                return;
            }

            std::stringstream buffer;
            buffer << ifs.rdbuf();
            std::string jsonInput = buffer.str();

            try
            {
                if (target == "disk")
                {
                    db_->importFromJsonToDisk(jsonInput, false);
                    db_->loadLogsIntoMemory();
                    std::cout << "Logs imported into disk table from " << filename << "\n";
                }
                else
                {
                    db_->importFromJson(jsonInput);
                    db_->save();
                    db_->loadLogsIntoMemory();
                    std::cout << "Logs imported into memory table from " << filename << "\n";
                }
            }
            catch (const std::exception &e)
            {
                std::cout << "Import failed: " << e.what() << "\n";
            }
        }

    private:
        MiniDB *db_;
    };
}