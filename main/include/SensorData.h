#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <string>

class SensorData {
   private:
    std::string device_id;
    std::string sensor_type;
    double sensor_value;
    std::string unit;
    std::string timestamp;

   public:
    SensorData(const std::string& device_id, const std::string& sensor_type,
               double sensor_value, const std::string& unit,
               const std::string& timestamp);

    std::string to_string();
};

#endif  // SENSORDATA_H
