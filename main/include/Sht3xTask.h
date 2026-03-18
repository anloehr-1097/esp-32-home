#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

struct ShtData {
  float temp;
  float hum;
};

class Sht3xTask {
private:
  TaskHandle_t task_handle;
  QueueHandle_t *queue;
  static void static_task_wrapper(void *pvParameter) {
    Sht3xTask *run_task = static_cast<Sht3xTask *>(pvParameter);
    run_task->task();
  }

public:
  Sht3xTask(QueueHandle_t *queue) {

    if (queue == NULL) {
      printf("Queue pointer on init is null\n");
      exit(1);
    }

    printf("Queue pointer on init not null\n");

    this->queue = queue;
    printf("Qeueue pointer: %p\n", queue);
  };

  void task();
  void register_task(const char *name, uint16_t stack_depth,
                     UBaseType_t priority);
};
