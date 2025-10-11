#include <FileChannel.hpp>
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>

namespace channel
{
    FileChannel::FileChannel(std::string path) : path_(path) {};

    void FileChannel::publish(const cppminidb::SensorLogRow &row) const
    {
        std::ofstream outFile(path_, std::ios::app); // append mode
        if (!outFile)
        {
            std::cerr << "[FileChannel] Failed to open file: " << path_ << std::endl;
            return;
        }

        outFile << row.toJSON().dump(4) << "\n";
    };
} // namespace channel