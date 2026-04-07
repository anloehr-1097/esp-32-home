/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_err.h"
#include <cstdint>
#include <inttypes.h>
#include <stdio.h>
// #include "freertos/projdefs.h"
#include "UpdateTask.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "esp_log_level.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "include/Sht3xTask.h"
#include "include/TempTask.h"
#include "include/WifiConnectTask.h"
#include "sdkconfig.h"
#include <memory>
#include <string>
#include <string_view>
// #include "driver/gpio.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "nvs_handle.hpp"
#include "secrets.h"
#include <cstdio>
// #include "soc/gpio_num.h"

EventGroupHandle_t wifi_event_group;

void set_string_in_nvs(nvs::NVSHandle *handle, const std::string &key,
                       const std::string &value) {
  // TODO(al) - when to set new values in NVS?
  // Write strings to NVS -> handle this later
  // err = handle->set_string("ssid", WIFI_SSID.c_str());
  // printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
  //
  // err_pwd = handle->set_string("password", WIFI_PASSWORD.c_str());
  // printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

  // Commit written value.
  // After setting any values, nvs_commit() must be called to ensure changes
  // are written to flash storage. Implementations may write to storage at
  // other times, but this is not guaranteed.
  // printf("Committing updates in NVS ... ");
  // err = handle->commit();
  // printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
  esp_err_t err = handle->set_string(key.data(), value.c_str());
  std::string tag = "NVS " + key;
  if (err != ESP_OK) {
    ESP_LOGE(tag.data(), "Error (%s) setting %s in NVS!\n",
             esp_err_to_name(err), key.data());
  } else {
    ESP_LOGI(tag.data(), "Successfully set %s in NVS.\n", key.data());
  }
}

std::string get_string_from_nvs(nvs::NVSHandle *handle, const std::string &key,
                                const std::string &default_value) {
  std::string tag = "NVS " + key;
  unsigned int out_len = 0;
  esp_err_t err;

  if ((err = handle->get_item_size(nvs::ItemType::SZ, key.data(), out_len)) ==
      ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGE(tag.data(), "%s not found in NVS. Using default value.\n",
             key.data());
    return default_value;
  } else if (err != ESP_OK) {
    ESP_LOGE(tag.data(), "Error (%s) getting %s size from NVS!\n",
             esp_err_to_name(err), key.data());
    return default_value;
  } else if (out_len > 0) {
    std::string out_str(out_len, '\0');
    err = handle->get_string(key.data(), out_str.data(), out_len);
    if (err != ESP_OK) {
      ESP_LOGE("NVS SSID", "Error (%s) reading SSID from NVS!\n",
               esp_err_to_name(err));
      return default_value;
    }
    return out_str;
  } else {
    return default_value;
  }
}

void get_device_info(void) {
  /* Print chip information */
  esp_chip_info_t chip_info;
  uint32_t flash_size;
  esp_chip_info(&chip_info);
  printf("This is %s chip with %d CPU core(s), %s%s%s%s, ", CONFIG_IDF_TARGET,
         chip_info.cores,
         (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
         (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
         (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
         (chip_info.features & CHIP_FEATURE_IEEE802154)
             ? ", 802.15.4 (Zigbee/Thread)"
             : "");

  unsigned major_rev = chip_info.revision / 100;
  unsigned minor_rev = chip_info.revision % 100;
  printf("silicon revision v%d.%d, ", major_rev, minor_rev);
  if (esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
    printf("Get flash size failed");
    return;
  }

  printf("%" PRIu32 "MB %s flash\n",
         flash_size / static_cast<uint32_t>(1024 * 1024),
         (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded"
                                                       : "external");

  printf("Minimum free heap size: %" PRIu32 " bytes\n",
         esp_get_minimum_free_heap_size());
}

std::unique_ptr<nvs::NVSHandle> initialize_nvs_flash() {
  // Initialize NVS
  esp_err_t err;
  err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  // Open
  ESP_LOGI("NVS Setup", "Opening Non-Volatile Storage (NVS) handle... ");
  // Handle will automatically close when going out of scope or when it's reset.
  // std::unique_ptr<nvs::NVSHandle> handle =
  std::unique_ptr<nvs::NVSHandle> handle =
      nvs::open_nvs_handle("storage", NVS_READWRITE, &err);
  if (err != ESP_OK) {
    ESP_LOGE("NVS Setup", "Error (%s) opening NVS handle!",
             esp_err_to_name(err));
  } else {
    ESP_LOGI("NVS Setup", "Done setting up NVS handle.");
  }

  return handle;
}

extern "C" void app_main(void) {
  std::unique_ptr<nvs::NVSHandle> nvs_handle = initialize_nvs_flash();
  std::string ssid(WIFI_SSID);
  std::string password(WIFI_PASSWORD);
  ssid = get_string_from_nvs(nvs_handle.get(), "ssid", WIFI_SSID);
  password = get_string_from_nvs(nvs_handle.get(), "password", WIFI_PASSWORD);

  wifi_event_group = xEventGroupCreate();
  constexpr size_t MAX_QUEUE_SIZE = 10;
  static QueueHandle_t queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(ShtData));
  if (queue == NULL) {
    ESP_LOGE("QueueCreate", "Failed to create queue\n");
    exit(1);
  }
  WifiConnectTask wifi_task = WifiConnectTask(ssid, password, wifi_event_group);
  wifi_task.register_task("WiFi Connect Task", 4096, 2 | portPRIVILEGE_BIT);

  Sht3xTask sht_task = Sht3xTask(&queue);
  sht_task.register_task("SHT3x task", 2048, 8);

  UpdateTask update_task = UpdateTask(wifi_event_group, &queue);
  update_task.register_task("UpdateTask", 4096, 4);

  // --- Temp sensor from HERE ---
  //
  // i2c_port_t port = I2C_NUM_0;
  //
  // const i2c_master_bus_config_t config = {
  //     .clk_source = I2C_CLK_SRC_DEFAULT,
  //     .glitch_ignore_cnt = 7,
  //     .sda_io_num = 21,
  //     .scl_io_num = 22,
  //     .i2c_port = port,
  //     .flags.enable_internal_pullup = 1
  // };
  //
  // i2c_master_bus_handle_t bus_hdl;
  // ESP_ERROR_CHECK(i2c_new_master_bus(&config, &bus_hdl));
  //
  // // i2c_bus_handle_t bus_hdl = i2c_bus_create(port, &conf);
  // if (bus_hdl == NULL) {
  //     printf("Failed to create I2C master bus\n");
  //     return;
  // }
  //
  //
  // // Add SHT3x device (addr 0x44 for VSS pin low)
  // i2c_device_config_t dev_cfg = {
  //     .dev_addr_length = I2C_ADDR_BIT_LEN_7,
  //     .device_address = 0x44,  // SHT3x_ADDR_PIN_SELECT_VSS
  //     .scl_speed_hz = 100000,
  // };
  // i2c_master_dev_handle_t sht_dev;
  // ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_hdl, &dev_cfg, &sht_dev));
  //
  //
  // float temp, hum = 0;
  // while (1) {
  //     // High precision measurement: send 0x2400, wait 15ms, fetch data
  //     uint8_t cmd[2] = {0x24, 0x00};
  //     uint8_t data[6];
  //
  //     ESP_ERROR_CHECK(i2c_master_transmit(sht_dev, cmd, 2, 30));
  //     ESP_ERROR_CHECK(i2c_master_receive(sht_dev, data, 6, 30));
  //     // printf("data: 0x%x %02X %02X %02X %02X %02X\n",
  //     data[0],data[1],data[2],data[3],data[4],data[5]);
  //
  //     uint16_t t_raw = (data[0] << 8) | data[1];
  //     uint16_t h_raw = (data[3] << 8) | data[4];
  //     // printf("Raw temperature: %d, Raw humidity: %d\n", t_raw, h_raw);
  //     temp = -45.0f + 175.0f * t_raw / 65535.0f;
  //     hum = 100.0f * (h_raw / 65535.0f);
  //     printf("Temperature: %.2f C, Humidity: %.2f %%\n", temp, hum);
  //     vTaskDelay(pdMS_TO_TICKS(1000 * 60 * 5 ));
  //

  // --- Temp sensor to HERE ---
  //
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
