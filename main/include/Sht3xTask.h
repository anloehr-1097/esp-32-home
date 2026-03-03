#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class Sht3xTask {
private:
  TaskHandle_t task_handle;

public:
  void task();
  void register_task(const char *name, uint16_t stack_depth,
                     UBaseType_t priority);
  static void static_task_wrapper(void *pvParameter) {
    Sht3xTask *run_task = static_cast<Sht3xTask *>(pvParameter);
    run_task->task();
  }
};
