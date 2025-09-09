#pragma once

#include "ICommand.hpp"
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

namespace cli
{
    class CommandRegistry
    {
    public:
        void registerCommand(std::unique_ptr<ICommand> cmd)
        {
            const std::string cmdName = cmd->name();
            if (commands_.find(cmdName) == commands_.end())
            {
                // std::move marks 'cmd' as an r-value, allowing its ownership to be transferred into the map without copying.
                // After this call, 'cmd' becomes invalid and should not be used.
                commands_.emplace(cmdName, std::move(cmd));
            }
        }
        void executeCommand(const std::string &cmdName, const std::vector<std::string> &args)
        {
            if (commands_.find(cmdName) != commands_.end())
            {
                commands_[cmdName]->execute(args);
            }
            else
            {
                std::cout << "Unknown command: " << cmdName << "\n";
            }
        }

    private:
        std::unordered_map<std::string, std::unique_ptr<ICommand>> commands_;
    };
} // namespace cli