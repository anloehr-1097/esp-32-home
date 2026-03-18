#include "include/UpdateTask.h"
#include "freertos/task.h"

void UpdateTask::task() {
  while (true) {
    printf("Update Task is running ...\n");
    vTaskDelay(pdMS_TO_TICKS(1000 * 3));
  }
}

void UpdateTask::register_task(const char *name, uint16_t stack_depth,
                               UBaseType_t priority) {
  xTaskCreate(static_task_wrapper, name, stack_depth, this, priority,
              &task_handle);
}
