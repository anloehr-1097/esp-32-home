#include "include/SensorData.h"

#include <fmt/format.h>

#include <string>

/*
 * SensorData class implementation.
 * This is the data structure representing a single sensor reading. After
 * serialziation, it can be sensor_type to the front end dashboard.
 */

SensorData::SensorData(const std::string& device_id,
                       const std::string& sensor_type, double sensor_value,
                       const std::string& unit, const std::string& timestamp)
    : device_id(device_id),
      sensor_type(sensor_type),
      sensor_value(sensor_value),
      unit(unit),
      timestamp(timestamp) {}

std::string SensorData::to_string() {
    return fmt::format(
        R"({{"device_id":"{}","sensor_type":"{}","sensor_value":{:.g},"unit":"{}","timestamp":"{}"}})",
        device_id, sensor_type, sensor_value, unit, timestamp);
}
