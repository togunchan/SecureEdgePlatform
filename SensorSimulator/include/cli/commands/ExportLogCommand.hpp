#pragma once

#include "ICommand.hpp"
#include "../EdgeShell.hpp"
#include "../../CppMiniDB/include/cppminidb/MiniDB.hpp"
#include <iostream>
#include <fstream>

namespace cli
{
    class ExportLogCommand : public ICommand
    {
    public:
        ExportLogCommand(MiniDB *db) : db_(db) {}

        std::string name() const override
        {
            return "exportlog";
        }

        void execute(const std::vector<std::string> &args) override
        {
            if (!db_)
            {
                std::cout << "Database is not initialized.\n";
                return;
            }

            std::string filename = "./data/logs.json";
            std::string source = "memory";

            for (const auto &arg : args)
            {
                if (arg.rfind("filename=", 0) == 0)
                {
                    filename = arg.substr(9);
                }
                else if (arg.rfind("source=", 0) == 0)
                {
                    source = arg.substr(7);
                }
            }

            std::string jsonOutput;
            if (source == "disk")
            {
                jsonOutput = db_->exportToJsonFromDisk();
            }
            else
            {
                jsonOutput = db_->exportToJson();
            }

            std::ofstream ofs(filename);
            if (!ofs)
            {
                std::cout << "Failed to open file: " << filename << "\n";
                return;
            }
            ofs << jsonOutput;
            ofs.close();

            std::cout << "Logs exported to " << filename
                      << " (source=" << source << ")\n";
        }

    private:
        MiniDB *db_;
    };
}