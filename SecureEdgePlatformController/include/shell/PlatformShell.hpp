#pragma once

#include <SecureEdgePlatformController.hpp>
#include <cli/EdgeShell.hpp>
#include <string>
#include <unordered_map>
#include <functional>

class PlatformShell
{
public:
    explicit PlatformShell(SecureEdgePlatformController &controller);
    void run();

private:
    SecureEdgePlatformController &controller_;
    std::unordered_map<std::string, std::function<void()>> commands_;
    sensor::EdgeShell edgeShell_;
    void registerCommands();
};
