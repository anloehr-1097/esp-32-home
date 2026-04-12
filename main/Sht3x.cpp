#include <cstdio>

#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "include/Sht3xTask.h"
#include "soc/gpio_num.h"

extern "C" void Sht3xTask::task() {
    i2c_port_t port = I2C_NUM_0;

    const i2c_master_bus_config_t config = {
        .i2c_port = port,
        .sda_io_num = (gpio_num_t)21,
        .scl_io_num = (gpio_num_t)22,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 20,
        .flags = {.enable_internal_pullup = 1, .allow_pd = 0}};

    i2c_master_bus_handle_t bus_hdl;
    ESP_ERROR_CHECK(i2c_new_master_bus(&config, &bus_hdl));

    // i2c_bus_handle_t bus_hdl = i2c_bus_create(port, &conf);
    if (bus_hdl == NULL) {
        printf("Failed to create I2C master bus\n");
        return;
    }

    // Add SHT3x device (addr 0x44 for VSS pin low)
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x44,  // SHT3x_ADDR_PIN_SELECT_VSS
        .scl_speed_hz = 100000,
        .scl_wait_us = 100,
        .flags = {.disable_ack_check = 0}};
    i2c_master_dev_handle_t sht_dev;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_hdl, &dev_cfg, &sht_dev));

    float temp, hum = 0;
    constexpr int sensor_setup_delay_ms = 15;
    vTaskDelay(pdMS_TO_TICKS(sensor_setup_delay_ms));

    while (1) {
        // High precision measurement: send 0x2400, wait 15ms, fetch data
        uint8_t cmd[2] = {0x24, 0x00};
        uint8_t data[6];

        taskDISABLE_INTERRUPTS();
        ESP_ERROR_CHECK(i2c_master_transmit(sht_dev, cmd, 2, 20));
        ESP_ERROR_CHECK(i2c_master_receive(sht_dev, data, 6, 20));
        taskENABLE_INTERRUPTS();

        uint16_t t_raw = (data[0] << 8) | data[1];
        uint16_t h_raw = (data[3] << 8) | data[4];
        // printf("Raw temperature: %d, Raw humidity: %d\n", t_raw, h_raw);
        temp = -45.0f + 175.0f * t_raw / 65535.0f;
        hum = 100.0f * (h_raw / 65535.0f);

        if (temp < -40.0f || temp > 125.0f || hum <= 0.0f || hum > 100.0f) {
            printf("Temperature reading out of range: %.2f C\n", temp);
            continue;
        }

        ShtData sht_data = {.temp = temp,
                            .hum = hum};  // might not work -> read up on this

        if (this->queue == NULL) {
            printf("Queue is NULL, cannot push temp & humidity data.\n");
        } else {
            if (xQueueSendToBack(*queue, &sht_data, 0) != pdPASS) {
                printf("Failed to push temp & humidty data to queue.\n");
            }
        }

        printf("Temperature: %.2f C, Humidity: %.2f %%\n", temp, hum);
        vTaskDelay(pdMS_TO_TICKS(30000 * 1));
    }
}

void Sht3xTask::register_task(const char* name, uint16_t stack_depth,
                              UBaseType_t priority) {
    xTaskCreate(static_task_wrapper, name, stack_depth, this, priority,
                &task_handle);
}
