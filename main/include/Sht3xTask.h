#ifndef SHT3XTASK_H
#define SHT3XTASK_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

struct ShtData {
    float temp;
    float hum;
};

class Sht3xTask {
   private:
    TaskHandle_t task_handle;
    QueueHandle_t* queue;
    const unsigned int record_frequency_ms;
    static void static_task_wrapper(void* pvParameter) {
        Sht3xTask* run_task = static_cast<Sht3xTask*>(pvParameter);
        run_task->task();
    }

   public:
    Sht3xTask(QueueHandle_t* queue, unsigned int record_frequency_ms)
        : record_frequency_ms(record_frequency_ms) {
        if (queue == NULL) {
            printf("Queue pointer on init is null\n");
            exit(1);
        }

        printf("Queue pointer on init not null\n");

        this->queue = queue;
        printf("Qeueue pointer: %p\n", queue);
    };

    void task();
    void register_task(const char* name, uint16_t stack_depth,
                       UBaseType_t priority);
};
#endif  // SHT3XTASK_H
