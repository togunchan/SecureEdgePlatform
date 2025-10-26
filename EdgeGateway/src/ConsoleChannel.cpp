#include <ConsoleChannel.hpp>
#include <iostream>

namespace channel
{
    void ConsoleChannel::publish(const cppminidb::SensorLogRow &row) const
    {
        std::cout << "[ConsoleChannel] Publishing row:\n";
        std::cout << row.toJSON().dump(4) << "\n";
    }
} // namespace channel