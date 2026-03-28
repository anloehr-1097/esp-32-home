#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "portmacro.h"
#include <clocale>
#include <format>
#include <string>

#include "esp_http_client.h"

class SensorData {
private:
  std::string device_id;
  std::string sensor_type;
  double sensor_value;
  std::string unit;
  std::string timestamp;

public:
  SensorData(const std::string &device_id, const std::string &sensor_type,
             double sensor_value, const std::string &unit,
             const std::string &timestamp);

  std::string to_string();
};

class UpdateTask {
private:
  TaskHandle_t task_handle;

  static void static_task_wrapper(void *pvParameter) {
    UpdateTask *run_task = static_cast<UpdateTask *>(pvParameter);
    run_task->task();
  }

public:
  void task();
  void register_task(const char *name, uint16_t stack_depth,
                     UBaseType_t priority);
};
