#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

extern "C" void task_function(void *pvParameter);
class TempTask {
private:
  TaskHandle_t task_handle;

public:
  friend void task_function(void *pvParameter);
  void task();
  void register_task(const char *name, uint16_t stack_depth,
                     UBaseType_t priority);
  static void static_task_wrapper(void *pvParameter) {
    TempTask *temp_task = static_cast<TempTask *>(pvParameter);
    temp_task->task();
  }
};
