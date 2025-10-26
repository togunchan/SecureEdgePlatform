#include <FileChannel.hpp>
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
#include <filesystem>
#include <system_error>

namespace channel
{
    FileChannel::FileChannel(std::string path) : path_(path) {};

    void FileChannel::publish(const cppminidb::SensorLogRow &row) const
    {
        const std::filesystem::path outputPath(path_);

        if (outputPath.has_parent_path())
        {
            std::error_code ec;
            std::filesystem::create_directories(outputPath.parent_path(), ec);
            if (ec)
            {
                std::cerr << "[FileChannel] Failed to create directories for: " << outputPath << " (" << ec.message() << ")\n";
                return;
            }
        }

        std::ofstream outFile(outputPath, std::ios::app); // append mode
        if (!outFile)
        {
            std::cerr << "[FileChannel] Failed to open file: " << outputPath << std::endl;
            return;
        }

        outFile << row.toJSON().dump(4) << "\n";
    };
} // namespace channel
