#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <clocale>

class WifiConnectTask {
private:
  TaskHandle_t task_handle;

  static void static_task_wrapper(void *pvParameter) {
    WifiConnectTask *run_task = static_cast<WifiConnectTask *>(pvParameter);
    run_task->task();
  }

public:
  void task();
  void register_task(const char *name, uint16_t stack_depth,
                     UBaseType_t priority);
};
