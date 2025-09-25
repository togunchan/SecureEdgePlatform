#include "ICommand.hpp"
#include "../EdgeShell.hpp"
#include "../../CppMiniDB/include/cppminidb/MiniDB.hpp"
#include <iostream>

namespace cli
{
    class QueryLogCommand : public ICommand
    {
    public:
        QueryLogCommand(MiniDB *db) : db_(db) {};

        std::string name() const override { return "querylog"; }

        void execute(const std::vector<std::string> &args) override
        {
            if (!db_)
            {
                std::cout << "Database is not initialized.\n";
                return;
            }

            std::vector<Condition> conditions;
            std::string source = "memory";

            for (size_t i = 0; i + 2 < args.size(); i += 3)
            {
                if (args[i].starts_with("column=") &&
                    args[i + 1].starts_with("op=") &&
                    args[i + 2].starts_with("value="))
                {

                    Condition cond;
                    cond.column = args[i].substr(7);
                    cond.op = args[i + 1].substr(3);
                    cond.value = args[i + 2].substr(6);
                    conditions.push_back(cond);
                }
            }

            for (const auto &arg : args)
            {
                if (arg.starts_with("source="))
                {
                    source = arg.substr(7);
                }
            }

            try
            {
                auto results = db_->selectWhereMulti(conditions, source == "disk");

                if (results.empty())
                {
                    std::cout << "No matching logs found.\n";
                    return;
                }

                std::cout << "Query Results:\n---------------------------------------------\n";
                for (const auto &row : results)
                {
                    std::cout << row.at("timestamp_ms") << "   "
                              << row.at("sensor_id") << "   "
                              << row.at("value") << "   "
                              << row.at("fault_flags") << "\n";
                }
                std::cout << "---------------------------------------------\n";
                std::cout << "Total: " << results.size() << " entries.\n";
            }
            catch (const std::exception &e)
            {
                std::cout << "Query error: " << e.what() << "\n";
            }
        }

    private:
        MiniDB *db_;
    };
}