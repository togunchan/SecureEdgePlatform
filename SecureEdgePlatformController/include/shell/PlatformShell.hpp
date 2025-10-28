#pragma once

#include <SecureEdgePlatformController.hpp>
#include <string>
#include <unordered_map>
#include <functional>
#include <iostream>

class PlatformShell
{
public:
    explicit PlatformShell(SecureEdgePlatformController &controller);
    void run();

private:
    SecureEdgePlatformController &controller_;
    std::unordered_map<std::string, std::function<void()>> commands_;

    void registerCommands();
};
