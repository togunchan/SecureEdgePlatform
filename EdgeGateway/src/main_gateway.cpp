#include <EdgeGateway.hpp>
#include <csignal>
#include <iostream>

namespace
{
    gateway::EdgeGateway *g_gateway = nullptr;

    void handleSignal(int signal)
    {
        if (signal == SIGINT)
        {
            std::cout << "\n[EdgeGatewayRunner] SIGINT received. Stopping loop...\n";
            if (g_gateway)
            {
                g_gateway->stopLoop();
            }
        }
    }
}

int main()
{
    gateway::EdgeGateway edgeGateway;
    g_gateway = &edgeGateway;
    std::signal(SIGINT, handleSignal);
    edgeGateway.start("");
    edgeGateway.runLoop();
    g_gateway = nullptr;

    return 0;
}
