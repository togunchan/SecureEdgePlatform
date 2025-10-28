#include <SecureEdgePlatformController.hpp>
#include <shell/PlatformShell.hpp>

int main(int argc, char *argv[])
{
    SecureEdgePlatformController controller;
    PlatformShell shell(controller);
    shell.run();
    return 0;
}