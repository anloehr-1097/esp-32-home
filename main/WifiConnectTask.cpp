#include "WifiConnectTask.h"

#include <algorithm>
#include <iostream>

#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_log_level.h"
#include "esp_netif_types.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "esp_wifi_types_generic.h"
#include "freertos/event_groups.h"
#include "freertos/idf_additions.h"
#include "include//helpers.h"
#include "include/helpers.h"
#define TAG "wifi task"

#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD "WPA2"
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER "NOIDEA"

void WifiConnectTask::task() {
    // default wifi stack initialization
    // TODO(al) this should be outsourced
    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // create default event loop and register event handler for wifi and ip
    // events
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_event_handler_instance_t instance_any_id;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID,
        [](void* arg, esp_event_base_t event_base, int32_t event_id,
           void* event_data) {
            WifiConnectTask* instance = static_cast<WifiConnectTask*>(arg);
            instance->event_handler(arg, event_base, event_id, event_data);
        },
        this, &instance_any_id));

    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP,
        [](void* arg, esp_event_base_t event_base, int32_t event_id,
           void* event_data) {
            WifiConnectTask* instance = static_cast<WifiConnectTask*>(arg);
            instance->event_handler(arg, event_base, event_id, event_data);
        },
        this, &instance_got_ip));

    // configure wifi settings
    wifi_config_t wifi_config = {
        .sta = {.ssid = {0},  // cannot assign the array, need to memcopy or
                              // std::copy, thus 0 init
                .password = {0}}};

    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.listen_interval = 1;
    esp_wifi_set_ps(WIFI_PS_NONE);

    std::copy(this->ssid.begin(), this->ssid.end(), wifi_config.sta.ssid);
    std::copy(this->password.begin(), this->password.end(),
              wifi_config.sta.password);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT)
     * or connection failed for the maximum number of re-tries (WIFI_FAIL_BIT).
     * The bits are set by event_handler.
     */
    EventBits_t bits =
        xEventGroupWaitBits(event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                            pdFALSE, pdFALSE, portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we
     * can test which event actually happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s", ssid.c_str());
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s", ssid.c_str());
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
    while (1) {
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
}

void WifiConnectTask::event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data) {
    static int s_retry_num = 0;
    esp_err_t err;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Connecting to Wifi - STA_START_EVENT received.");
        ESP_ERROR_CHECK((err = esp_wifi_connect()));
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to connect to WiFi: %s",
                     esp_err_to_name(err));
        }
        ESP_LOGI(TAG, "connect to the AP successfully initiated");

    } else if (event_base == WIFI_EVENT &&
               event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG,
                 "Disconnecting from Wifi - STA_DISCONNECTED_EVENT received.");
        uint8_t* reason = reinterpret_cast<uint8_t*>(event_data);
        ESP_LOGI(TAG, "reason: %d", *reason);

        if (*reason == WIFI_REASON_BEACON_TIMEOUT) {
            ESP_LOGE(TAG, "BEACON TIMEOUT while  connecting to WiFi");
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }

        if (s_retry_num < max_retry_num) {
            ESP_LOGI(TAG, "retry to connect to the AP");
            err = esp_wifi_connect();
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to connect to WiFi: %s",
                         esp_err_to_name(err));
            }
            s_retry_num++;
        } else {
            xEventGroupSetBits(event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event =
            reinterpret_cast<ip_event_got_ip_t*>(event_data);
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(event_group, WIFI_CONNECTED_BIT);
    }
}

void WifiConnectTask::register_task(const char* name, uint16_t stack_depth,
                                    UBaseType_t priority) {
    xTaskCreate(static_task_wrapper, name, stack_depth, this, priority,
                &task_handle);
}
