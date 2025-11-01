#include <shell/PlatformShell.hpp>
#include <cli/commands/CommandRegistry.hpp>

PlatformShell::PlatformShell(SecureEdgePlatformController &controller) : controller_(controller)
{
    edgeShell_.setScheduler(&controller_.getGateway().getScheduler());
    registerCommands();
}

void PlatformShell::registerCommands()
{
    commands_.emplace("boot", [this]()
                      {
                        const bool success = controller_.bootPhase();
                        if (!success)
                            std::cerr << "[Shell] Boot command failed.\n"; });
    commands_.emplace("start", [this]()
                      { controller_.start(); });
    commands_.emplace("stop", [this]()
                      { controller_.stop(); });
    commands_.emplace("status", [this]()
                      { std::cout << "[Shell] System running.\n"; });
    commands_.emplace("flush", [this]()
                      { std::cout << "[Shell] (TODO) flush logs\n"; });
    commands_.emplace("exit", [this]()
                      { controller_.stop();
                        std::cout << "[Shell] Exiting...\n";
                        std::exit(0); });
    commands_.emplace("sensors", [this]()
                      {
                        std::cout << "[Shell] Entering Sensor Management Mode...\n";
                        controller_.stop();
                        edgeShell_.run(sensor::Mode::Restricted);
                        std::cout << "[Shell] Exited Fault Injection Mode.\n"; });
}

void PlatformShell::run()
{
    std::string input;
    std::cout << "[SecureEdgePlatform Shell] Type 'help' for commands.\n";

    while (true)
    {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, input))
            break;

        if (input == "help")
        {
            std::cout << "Available commands:\n";
            for (auto &[cmd, _] : commands_)
            {
                std::cout << " " << cmd << "\n";
            }
            continue;
        }

        auto it = commands_.find(input);
        if (it != commands_.end())
        {
            std::cout << "Selected operation: " << it->first << "\n";
            it->second();
        }
        else
        {
            std::cout << "Unknown command: " << input << "\n";
        }
    }
}
