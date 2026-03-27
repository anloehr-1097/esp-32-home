#include "include/UpdateTask.h"
#include "freertos/task.h"

#include "esp_http_client.h"
#include "esp_log.h"
#include <fmt/format.h>
#include <iostream>
#include <string>

static const char *TAG = "HTTP_CLIENT";

SensorData::SensorData(const std::string &device_id,
                       const std::string &sensor_type, double sensor_value,
                       const std::string &unit, const std::string &timestamp)
    : device_id(device_id), sensor_type(sensor_type),
      sensor_value(sensor_value), unit(unit), timestamp(timestamp) {}

std::string SensorData::to_string() const {
  return fmt::format(
      R"({{"device_id":"{}","sensor_type":"{}","sensor_value":{},"unit":"{}","timestamp":"{}"}})",
      device_id, sensor_type, sensor_value, unit, timestamp);
}

void UpdateTask::task() {
  /*
   * Need client config
   * event handler
   */

  const char *url_to_push_to = "http://localhost";
  // int port = 8080;
  // const char *host = "localhost";
  constexpr uint MAX_HTTP_OUTPUT_BUFFER = 2000;

  // fake data
  SensorData data("device123", "temperature", 255.0, "Celsius",
                  "2024-06-27T12:00:00Z");

  std::cout << data.to_string() << std::endl;
  // esp_http_client_config_t config = {.url = url_to_push_to, .port = port};
  // esp_http_client_handle_t client = esp_http_client_init(&config);

  // **** THIS IS THE FIELDS EXPECTED ****

  // // Extract data from JSON
  // std::string device_id = json["device_id"].s();
  // std::string sensor_type = json["sensor_type"].s();
  // double sensor_value = json["sensor_value"].d();
  // std::string unit = json.has("unit") ? std::string(json["unit"].s()) :
  // std::string(""); std::string timestamp = json.has("timestamp") ?
  // std::string(json["timestamp"].s()) : std::string("");

  // Declare local_response_buffer with size (MAX_HTTP_OUTPUT_BUFFER + 1) to
  // prevent out of bound access when it is used by functions like strlen(). The
  // buffer should only be used upto size MAX_HTTP_OUTPUT_BUFFER
  char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER + 1] = {0};
  /**
   * NOTE: All the configuration parameters for http_client must be specified
   * either in URL or as host and path parameters. If host and path parameters
   * are not set, query parameter will be ignored. In such cases, query
   * parameter should be specified in URL.
   *
   * If URL as well as host and path parameters are specified, values of host
   * and path will be considered.
   */
  esp_http_client_config_t config = {
      .url = url_to_push_to,
      .host = url_to_push_to,
      .port = 8080,
      .path = "/api/data",
      .user_data =
          local_response_buffer, // Pass address of local buffer to get response
                                 //.disable_auto_redirect = true,
  };
  esp_http_client_handle_t client = esp_http_client_init(&config);

  // POST
  const char *post_data = data.to_string().c_str();
  esp_http_client_set_url(client, "http://localhost:8080/api/data");
  esp_err_t err;
  esp_http_client_set_method(client, HTTP_METHOD_POST);
  esp_http_client_set_header(client, "Content-Type", "application/json");
  esp_http_client_set_post_field(client, post_data, strlen(post_data));
  err = esp_http_client_perform(client);
  if (err == ESP_OK) {
    ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %" PRId64,
             esp_http_client_get_status_code(client),
             esp_http_client_get_content_length(client));
  } else {
    ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
  }

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
