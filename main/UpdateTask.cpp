#include "include/UpdateTask.h"
#include "esp_err.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"

#include "esp_http_client.h"
#include "esp_log.h"
#include "include/Sht3xTask.h"
#include "include/helpers.h"
#include <chrono>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <iostream>
#include <string>

static const char *TAG = "HTTP_CLIENT";

SensorData::SensorData(const std::string &device_id,
                       const std::string &sensor_type, double sensor_value,
                       const std::string &unit, const std::string &timestamp)
    : device_id(device_id), sensor_type(sensor_type),
      sensor_value(sensor_value), unit(unit), timestamp(timestamp) {}

std::string SensorData::to_string() {
  return fmt::format(
      R"({{"device_id":"{}","sensor_type":"{}","sensor_value":{:.g},"unit":"{}","timestamp":"{}"}})",
      device_id, sensor_type, sensor_value, unit, timestamp);
}

void UpdateTask::task() {
  /*
   * Need client config
   * event handler
   */

  const char *url_to_push_to = "http://192.168.178.55";
  // int port = 8080;
  // const char *host = "localhost";
  // constexpr unsigned int MAX_HTTP_OUTPUT_BUFFER = 1024;

  // fake data
  SensorData data("device123", "temperature", 255.0, "Celsius",
                  "2024-06-27T12:00:00Z");

  auto json_data = data.to_string();
  std::cout << data.to_string() << std::endl;

  esp_http_client_config_t config = {
      .url = url_to_push_to,
      // .host = url_to_push_to,
      // .port = 8080,
      //.path = "/api/data",
      .timeout_ms = 15000,
      // .user_data =
      //     local_response_buffer, // Pass address of local buffer to get
      //     response
      //.disable_auto_redirect = true,
  };
  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (client == NULL) {
    ESP_LOGE(TAG, "Failed to initialize HTTP client");
    return;
  }
  while (1) {
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
    // prevent out of bound access when it is used by functions like strlen().
    // The buffer should only be used upto size MAX_HTTP_OUTPUT_BUFFER char
    // local_response_buffer[MAX_HTTP_OUTPUT_BUFFER + 1] = {0};
    /**
     * NOTE: All the configuration parameters for http_client must be specified
     * either in URL or as host and path parameters. If host and path parameters
     * are not set, query parameter will be ignored. In such cases, query
     * parameter should be specified in URL.
     *
     * If URL as well as host and path parameters are specified, values of host
     * and path will be considered.
     */

    // UBaseType_t watermark = uxTaskGetStackHighWaterMark(NULL);
    // ESP_LOGI("STACK", "High watermark: %d words remaining", watermark);
    //

    EventBits_t bits =
        xEventGroupWaitBits(event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                            pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
      ShtData data_buffer;
      xQueueReceive(*queue, &data_buffer, pdMS_TO_TICKS(1000));
      if (data_buffer.temp == 0 && data_buffer.hum == 0) {
        ESP_LOGW(TAG,
                 "Received default ShtData from queue, skipping HTTP POST");
        continue;
      }
      // create data from reading to pass to server
      std::chrono::system_clock::time_point now =
          std::chrono::system_clock::now();
      std::string this_timestamp = fmt::format("{:%Y-%m-%dT%H:%M:%SZ}", now);

      SensorData data("Sht3x - demo", "temperature", data_buffer.temp,
                      "Celsius", this_timestamp.c_str());

      // POST
      // const char *post_data = data.to_string().c_str();
      esp_http_client_set_method(client, HTTP_METHOD_POST);
      esp_http_client_set_url(client, "http://192.168.178.55:8080/api/data");

      std::string payload = data.to_string();
      esp_http_client_set_header(client, "Content-Type", "application/json");
      esp_err_t err = esp_http_client_set_post_field(client, NULL, 0);
      ESP_LOGE(TAG, "Setting post field with NULL data: %s",
               esp_err_to_name(err));
      err = esp_http_client_set_post_field(client, payload.c_str(),
                                           payload.length());

      ESP_LOGE(TAG, "Setting post field with actual data: %s yields error %s",
               data.to_string().c_str(), esp_err_to_name(err));

      ESP_LOGI(TAG, "WIFI_CONNECTED_BIT set, making http request");

      // esp_http_client_set_method(client, HTTP_METHOD_GET);
      // esp_http_client_set_url(client, "http://192.168.178.55/healthcheck");
      err = esp_http_client_perform(client);

      if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %" PRId64,
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
      } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
      }
    } else if (bits & WIFI_FAIL_BIT) {
      ESP_LOGI(TAG, "Failed to connect to WiFi, cannot perform HTTP POST");
    } else {
      ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    // while (true) {
    //   printf("Update Task is running ...\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
    // }
  }
}

void UpdateTask::register_task(const char *name, uint16_t stack_depth,
                               UBaseType_t priority) {
  xTaskCreate(static_task_wrapper, name, stack_depth, this, priority,
              &task_handle);
}
