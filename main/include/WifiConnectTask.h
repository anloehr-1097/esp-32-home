#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include <clocale>
#include <string>

class WifiConnectTask {

public:
  WifiConnectTask(std::string ssid, std::string password,
                  EventGroupHandle_t &event_group)
      : ssid(ssid), password(password), event_group(event_group) {}

private:
  TaskHandle_t task_handle;
  std::string ssid;
  std::string password;
  int max_retry_num = 10;
  EventGroupHandle_t &event_group;
  void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                     void *event_data);
  static void static_task_wrapper(void *pvParameter) {
    WifiConnectTask *run_task = static_cast<WifiConnectTask *>(pvParameter);
    run_task->task();
  }

public:
  void task();
  void register_task(const char *name, uint16_t stack_depth,
                     UBaseType_t priority);
};
