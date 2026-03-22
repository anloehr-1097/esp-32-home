#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "portmacro.h"
#include <clocale>

#include "esp_http_client.h"

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
