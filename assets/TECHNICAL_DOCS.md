# Technical Documentation: ESP32 Environmental Monitor

This document outlines the software architecture, design decisions, and system capabilities of the ESP32 Environmental Monitor. It is intended to provide technical context for engineers and reviewers looking at the codebase.

## System Architecture

The project is built on the **ESP-IDF** framework using **FreeRTOS** for real-time task scheduling and **Modern C++** (C++17/C++20 features) for object-oriented encapsulation and resource management.

The system is designed around a decoupled, producer-consumer architecture, ensuring that blocking network operations do not interrupt sensor readings.

### Core Components & Tasks

The application logic is separated into distinct, concurrent FreeRTOS tasks:

1. **`Sht3xTask` (Producer)**
   * **Responsibility**: Interfaces with the SHT3x temperature and humidity sensor via the I2C bus.
   * **Execution**: Runs periodically based on `TEMP_RECORD_FREQUENCY_MS`.
   * **Data Handling**: Wraps the raw sensor readings into a `ShtData` struct and dispatches it to a thread-safe FreeRTOS Queue (`xQueue`).

2. **`UpdateTask` (Consumer & Network Client)**
   * **Responsibility**: Listens to the FreeRTOS Queue for new `ShtData` and pushes the telemetry to a remote dashboard via HTTP POST.
   * **Synchronization**: Uses a FreeRTOS `EventGroup` to monitor the system's network state. It suspends HTTP operations and prevents queue consumption if the `WIFI_CONNECTED_BIT` is not set, preventing network timeouts from hanging the system.
   * **Data Formatting**: Utilizes `fmt` to format UTC timestamps and constructs JSON payloads dynamically.

3. **`WifiConnectTask` (Network Manager)**
   * **Responsibility**: Manages the Wi-Fi station lifecycle (initialization, connection, and reconnection strategies).
   * **State Management**: Updates the FreeRTOS `EventGroup` (`WIFI_CONNECTED_BIT` or `WIFI_FAIL_BIT`) to broadcast network state changes to other tasks asynchronously.

### Non-Volatile Storage (NVS)
The system utilizes the ESP32's NVS partition to store and retrieve persistent configuration, such as Wi-Fi credentials (`ssid`, `password`).
