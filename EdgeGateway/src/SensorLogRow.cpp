#include <cppminidb/SensorLogRow.hpp>

namespace cppminidb
{
    nlohmann::json SensorLogRow::toJSON() const
    {
        return {
            {"timestamp", timestamp_ms},
            {"sensorId", sensor_id},
            {"value", value},
            {"faultType", fault_flags}};
    }
} // namespace cppminidb