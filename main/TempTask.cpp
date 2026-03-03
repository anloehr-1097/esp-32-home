#include "include/TempTask.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <functional>

void task_function(void *pvParameter) {
  TempTask *temp_task = static_cast<TempTask *>(pvParameter);
  temp_task->task();
}

void TempTask::task() {
  while (true) {
    printf("Temp Task is running ...\n");
    vTaskDelay(pdMS_TO_TICKS(1000 * 3));
  }
}

void TempTask::register_task(const char *name, uint16_t stack_depth,
                             UBaseType_t priority) {
  xTaskCreate(static_task_wrapper, name, stack_depth, this, priority,
              &task_handle);
}
