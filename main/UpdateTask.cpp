#include "include/UpdateTask.h"
#include "freertos/task.h"

#include "esp_http_client.h"
#include <string>

void UpdateTask::task() {

  const char *url_to_push_to = "http://localhost";
  int port = 8080;
  const char *host = "localhost";

  esp_http_client_config_t config = {.url = url_to_push_to, .port = port};
  esp_http_client_handle_t client = esp_http_client_init(&config);

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
