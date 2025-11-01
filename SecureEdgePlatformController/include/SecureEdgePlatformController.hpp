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
    gateway::EdgeGateway &getGateway();
    const gateway::EdgeGateway &getGateway() const;

private:
    friend class PlatformShell;

    void runLoop();
    bool bootPhase();
    gateway::EdgeGateway gateway_;
    edgeagent::EdgeAgent agent_;

    std::atomic<bool> running_{false};
    std::thread loopThread_;
};
