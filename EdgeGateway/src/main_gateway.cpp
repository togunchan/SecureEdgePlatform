#include <EdgeGateway.hpp>

int main()
{
    gateway::EdgeGateway edgeGateway;
    edgeGateway.start();
    edgeGateway.runLoop();

    return 0;
}
