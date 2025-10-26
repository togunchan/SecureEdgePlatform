#pragma once
#include <EdgeGateway.hpp>
#include <edgeagent/EdgeAgent.hpp>
#include <memory>
#include <atomic>
#include <thread>

class SecureEdgePlatformController
{
public:
    SecureEdgePlatformController();
    ~SecureEdgePlatformController();

    void start();
    void stop();

private:
    void runLoop();
    gateway::EdgeGateway gateway_;
    edgeagent::EdgeAgent agent_;

    std::atomic<bool> running_{false};
    std::thread loopThread_;
};