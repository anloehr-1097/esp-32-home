#include "esp_log.h"
#include "esp_sntp.h"

void time_sync_callback(struct timeval* tv) {
    ESP_LOGI("NTP", "Time synchronized!");
}

void init_sntp(void) {
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_callback);
    esp_sntp_init();

    // Wait for sync
    int retry = 0;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < 10) {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
