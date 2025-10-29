#include <shell/PlatformShell.hpp>
#include <cli/commands/ListCommand.hpp>
#include <cli/commands/CommandRegistry.hpp>

PlatformShell::PlatformShell(SecureEdgePlatformController &controller) : controller_(controller)
{
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

    cli::CommandRegistry registry;
    registry.registerCommand(std::make_unique<cli::ListCommand>(edgeShell_));
    std::cout << "[Shell] Integrated ListCommand from SensorSimulator.\n";
}

void PlatformShell::run()
{
    std::string input;
    std::cout << "[SecureEdgePlatform Shell] Type 'help' for commands.\n";

    while (true)
    {
        std::cout << "> ";
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