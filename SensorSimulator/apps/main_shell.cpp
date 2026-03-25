#include "cli/EdgeShell.hpp"

int main()
{
    sensor::EdgeShell shell;
    MiniDB db("sensor_simulator_logs");
    shell.setDatabase(&db);
    shell.run();
    return 0;
}
