#pragma once

#include <IGatewayChannel.hpp>

namespace channel
{
    class FileChannel : public IGatewayChannel
    {
    public:
        explicit FileChannel(std::string path);

        void publish(const cppminidb::SensorLogRow &row) const override;

    private:
        std::string path_;
    };
} // namespace channel