#include <clocale>
#include <string>

#include "esp_event.h"  // needed for esp_event_base_t
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "helpers.h"

/*
 * Class responsible for wifi connection.
 * Uses event group to signal other tasks status of wifi connection.
 */
class WifiConnectTask {
   private:
    TaskHandle_t task_handle;
    std::string ssid;
    std::string password;
    unsigned int max_retry_num = MAX_WIFI_RETRY_NUM;
    EventGroupHandle_t event_group;
    void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id,
                       void* event_data);
    static void static_task_wrapper(void* pvParameter) {
        WifiConnectTask* run_task = static_cast<WifiConnectTask*>(pvParameter);
        run_task->task();
    }

   public:
    WifiConnectTask(std::string ssid, std::string password,
                    EventGroupHandle_t event_group)
        : ssid(ssid), password(password), event_group(event_group) {}
    void task();
    void register_task(const char* name, uint16_t stack_depth,
                       UBaseType_t priority);
};
