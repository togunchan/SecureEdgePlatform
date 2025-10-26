#include <SecureEdgePlatformController.hpp>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>

std::atomic<bool> running{true};

void handleSignal(int)
{
    running.store(false);
}

int main(int argc, char *argv[])
{
    std::signal(SIGINT, handleSignal);

    if (argc < 2)
    {
        std::cerr << "Usage: ./SecureEdgePlatformRunner [start|stop|flush]\n";
        return 1;
    }

    std::string command = argv[1];
    SecureEdgePlatformController controller;

    if (command == "start")
    {
        controller.start();
        std::cout << "[Runner] Press Ctrl+C to stop.\n";

        while (running.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        controller.stop();
        std::cout << "[Runner] Stopped cleanly.\n";
    }
    else if (command == "stop")
    {
        controller.stop();
    }
    else
    {
        std::cerr << "Unknown command: " << command << "\n";
    }

    return 0;
}