#include "include/UpdateTask.h"

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <sys/select.h>

#include <chrono>
#include <ctime>
#include <string>

#include "HttpClient.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "include/SensorData.h"
#include "include/Sht3xTask.h"
#include "include/helpers.h"

static const char* TAG = "UpdateTask";

HttpClient UpdateTask::setup() { return HttpClient(push_url.c_str()); }

void UpdateTask::push_data_to_server(const ShtData& data) {
    if (!client.has_value()) {
        ESP_LOGE(TAG, "HTTP client is not initialized");
        return;
    }

    if (client->clear_post_field() != ESP_OK) {
        return;
    }

    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm utc_time = *std::gmtime(&time);

    std::string this_timestamp = fmt::format("{:%Y-%m-%dT%H:%M:%SZ}", utc_time);

    if (client->set_method(HTTP_METHOD_POST) != ESP_OK) {
        return;
    }

    if (client->set_url(push_url.c_str()) != ESP_OK) {
        return;
    }

    if (client->set_header("Content-Type", "application/json") != ESP_OK) {
        return;
    }

    // push data to server
    SensorData sensor_data("Sht3-office", "temperature", data.temp, "Celsius",
                           this_timestamp.c_str());
    std::string payload = sensor_data.to_string();
    if (client->set_post_field(payload.c_str(), payload.length()) != ESP_OK) {
        return;
    }
    if (client->perform() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to perform HTTP POST request");
        return;
    }

    sensor_data = {"Sht3-office", "humidity", data.hum, "%",
                   this_timestamp.c_str()};

    payload = sensor_data.to_string();
    if (client->set_post_field(payload.c_str(), payload.length()) != ESP_OK) {
        return;
    }
    if (client->perform() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to perform HTTP POST request");
        return;
    }
    //
    // if (err == ESP_OK) {
    //     ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %" PRId64,
    //              esp_http_client_get_status_code(client),
    //              esp_http_client_get_content_length(client));
    // } else {
    //     ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    // }
}

void UpdateTask::task() {
    /*
     * event handler
     */
    client = setup();
    ShtData data;

    while (1) {
        // only perform HTTP POST if connected to WiFi.
        // if not connected, don't consume from Queue.

        EventBits_t bits =
            xEventGroupWaitBits(event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                pdFALSE, pdFALSE, portMAX_DELAY);

        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "WIFI_CONNECTED_BIT set, making http request");
            if (xQueueReceive(queue, &data, pdMS_TO_TICKS(1000))) {
                push_data_to_server(data);
                // push data to server
            }
            // create data from reading to pass to server
            // POST
            // const char *post_data = data.to_string().c_str();
        } else if (bits & WIFI_FAIL_BIT) {
            ESP_LOGI(TAG,
                     "Failed to connect to WiFi, cannot perform HTTP POST");
        } else {
            ESP_LOGE(TAG, "UNEXPECTED EVENT");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void UpdateTask::register_task(const char* name, uint16_t stack_depth,
                               UBaseType_t priority) {
    xTaskCreate(static_task_wrapper, name, stack_depth, this, priority,
                &task_handle);
}
