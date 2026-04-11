#ifndef MAIN_INCLUDE_UPDATETASK_H_
#define MAIN_INCLUDE_UPDATETASK_H_

#include <clocale>
#include <optional>
#include <string>

#include "HttpClient.h"
#include "Sht3xTask.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "portmacro.h"

class UpdateTask {
   private:
    TaskHandle_t task_handle;
    EventGroupHandle_t event_group;
    QueueHandle_t queue;
    std::string push_url;
    std::optional<HttpClient> client;

    static void static_task_wrapper(void* pvParameter) {
        UpdateTask* run_task = static_cast<UpdateTask*>(pvParameter);
        run_task->task();
    }
    HttpClient setup();
    void push_data_to_server(const ShtData& data);

   public:
    UpdateTask(EventGroupHandle_t event_group, QueueHandle_t queue,
               std::string push_url)
        : event_group(event_group), queue(queue), push_url(push_url) {}

    void task();
    void register_task(const char* name, uint16_t stack_depth,
                       UBaseType_t priority);
};

#endif  // MAIN_INCLUDE_UPDATETASK_H_
