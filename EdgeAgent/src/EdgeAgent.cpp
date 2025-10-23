#pragma once

#include <edgeagent/EdgeAgent.hpp>
#include <iostream>

namespace edgeagent
{
    void EdgeAgent::receive(const cppminidb::SensorLogRow &row)
    {
        buffer_.push_back(row);
    }

    void EdgeAgent::flushToConsole()
    {
        if (buffer_.empty())
        {
            std::cout << "[EdgeAgent] No data to flush to console.\n";
            return;
        }

        publisher_.publishToConsole(buffer_);
        buffer_.clear();
    }

    void EdgeAgent::flushToFile(const std::string &filename)
    {
        if (buffer_.empty())
        {
            std::cerr << "[EdgeAgent] No data to flush to file: " << filename << "\n";
            return;
        }

        try
        {
            publisher_.publishToFile(buffer_, filename);
            buffer_.clear();
        }
        catch (const std::exception &ex)
        {
            std::cerr << "[EdgeAgent] Failed to flush to file: " << ex.what() << "\n";
        }
    }
} // namespace edgeagent