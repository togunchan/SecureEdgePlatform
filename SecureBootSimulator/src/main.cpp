#include <secureboot/BootConfig.hpp>
#include <secureboot/BootSimulator.hpp>

#include <iostream>
#include <thread>
#include <random>

using namespace secureboot;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <config_path>" << std::endl;
        return 1;
    }

    const std::string configPath = argv[1];
    BootConfig config;
    if (!config.loadFromFile(configPath))
    {
        std::cerr << "[SecureBoot] Failed to load config from: " << configPath << std::endl;
        return 2;
    }

    BootSimulator simulator(config);

    simulator.addStage(BootStage("VerifyPowerRails", 1, []()
                                 {
                                     std::cout << "[Stage] Checking power rails...";

                                     // simulate delay
                                     std::this_thread::sleep_for(std::chrono::milliseconds(150));

                                     // simulate random voltage check
                                     std::random_device rd;
                                     std::mt19937 gen(rd());
                                     std::uniform_int_distribution<> voltage(3200, 3300); //millivolts

                                     int measuredVoltage = voltage(gen);
                                     std::cout << " measured " << measuredVoltage << " mV\n";

                                     if (measuredVoltage < 3250)
                                     {
                                         throw StageFailure(101, "Undervoltage detected on power rail");
                                     } }));

    simulator.addStage(BootStage("LoadFirmware", 2, []()
                                 { std::cout << "[Stage] LoadFirmware: done." << std::endl; }));

    simulator.addStage(BootStage("JumpToEntry", 3, []()
                                 { std::cout << "[Stage] JumpToEntry: done." << std::endl; }));

    simulator.run();

    if (simulator.wasSuccessful())
    {
        std::cout << "[SecureBoot] Boot simulation completed successfully." << std::endl;
        return 0;
    }
    else
    {
        std::cerr << "[SecureBoot] Boot simulation failed: " << simulator.getFailureReason() << std::endl;
        return 3;
    }

    return 0;
}