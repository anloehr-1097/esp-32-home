# Home automation Example for the ESP32 using the ESP-IDF framework.

This repo illustrates how the ESP-IDF framework can be used to read temperature and humidity data from a SHT3x sensor
and pushed to a dashboard running in the same local network via http over a Wifi connection.

This is a hobby project, thus the code is not production ready. It is however a good starting point to iterate upon.

# Software Components
The project is built on the **ESP-IDF** framework using **FreeRTOS** for real-time task scheduling and **Modern C++** (C++17/C++20 features) for object-oriented encapsulation and resource management.

# Hardware Components
The following hardware components are required to replicate: 

- Esp32 (any version with wifi should do, I used the ESP32-WROOM-32U)
- SHT3x sensor - specifically, I used the SHT31-D sensor
- The usual peripherals like a breadboard, jumper wires, a USB cable to flash the ESP32, etc.
- (A dashboard to push the data to. This can be ommitted if you just want to print the readings to the serial console)


# Design Overview
The design of the system is based on a modular architecture, where each component is responsible for a specific functionality (task in FreeRTOS jargon). 

The main tasks are:
- Wifi Task: responsible for connecting to the wifi network and maintaining the connection
- Sensor Task: responsible for reading data from the SHT3x sensor at regular time intervals and pushing them to a queue
- Update Task: responsible for pushing the data to the dashboard via HTTP POST requests

Additionally to manage depedencies between the tasks, I use the following synchronization primitives:
- Queue: A static queue used for communication between the Sensor Task and the Update Task.
- Event Group: An event group used to coordinate Update task and Wifi task.


All of the above tasks follow a similar pattern. They are wrapped in classes that encapsulate  the functionality and state of the task.
Each class has a `task()` member function that contains the task logic. 
The `register_task()` member function wraps the FreeRTOS `xTaskCreate()` function setting task parameters, priority etc and starts the task.
As the `xTaskCreate()` function requires a non-member function as the task entry point,
I use a static function `static_task_wrapper()` in the namespace of the respective class that serves as a wrapper to call the non-static `task()` member function
passing the entire class instance as a parameter.

In effect, the pattern to create and start a task is as follows:
```cpp

// Create sensor task with static task queue and read frequency as parameters
Sht3xTask sht_task = Sht3xTask(&queue, TEMP_RECORD_FREQUENCY_MS);
// register task with stack size of 2048 bytes and priority of 8
sht_task.register_task("SHT3x task", 2048, 8);

```

## Wifi Task
   * **Responsibility**: Manages the Wi-Fi station lifecycle (initialization, connection, and reconnection strategies).
   * **State Management**: Updates the FreeRTOS `EventGroup` (`WIFI_CONNECTED_BIT` or `WIFI_FAIL_BIT`) to broadcast network state changes to other tasks asynchronously.
   * **Credential Handling**: Wifi credential are retrieved from the Non-Volatile Storage (NVS) at startup.  If not available under the corresponding keys, the system falls back to credentials found in a `secret.h` file (which is not committed to the repository for security reasons). This allows for flexible configuration without hardcoding sensitive information in the codebase.

## Sensor Task
   * **Responsibility**: Interfaces with the SHT3x temperature and humidity sensor via the I2C bus.
   * **Execution**: Runs periodically based on `TEMP_RECORD_FREQUENCY_MS`.
   * **Data Handling**: Wraps the raw sensor readings into a `ShtData` struct and dispatches it to a thread-safe FreeRTOS Queue (`xQueue`).

## Update Task
   * **Responsibility**: Listens to the FreeRTOS Queue for new `ShtData` and pushes the telemetry to a remote dashboard via HTTP POST.
   * **Synchronization**: Uses a FreeRTOS `EventGroup` to monitor the system's network state. It suspends HTTP operations and prevents queue consumption if the `WIFI_CONNECTED_BIT` is not set, preventing network timeouts from hanging the system.
   * **Data Formatting**: Utilizes `fmt` to format UTC timestamps and constructs JSON payloads dynamically.

## Auxiliary Components



# Build and Deployment

This project requires the ESP-IDF toolchain. Refer to the official ESP-IDF documentation for setup instructions.

1. Ensure your ESP-IDF environment is sourced. 
2. Build the project: `idf.py build`
3. Flash and monitor: `idf.py flash monitor` (make sure to select correct port, usually /dev/ttyUSB0 or similar)

# Next Steps
- [] Complete the wifi config: 
    - [x] Retrieve & store credentials
    - [x] Setup event group s.t. the update task is only pushing once the wifi connection is established
    - [-] Setup Event handler to handle wifi events (e.g. disconnection, reconnection, etc.)
    - [x] Debug why wifi not working - isolated task works, not in combination with the rest of the tasks. Maybe need to delay udpate task sensibly
       -> One issue was actually weak signal strength

    - [ ] Cleanup 

- [ ] Test update / pushing to dashboard
    - [x] Probably formatting not correct of test data pushed --> solved formatting / string was only temporary, went out of bounds, thus c_str as well
    - [x] register http event handler, see docs

- [x] Complete the update task:
    - [x] Push data to the dashboard using HTTP POST requests
    - [x] Retrieve the data from the queue 


- [x] SHT3x
    - [x] Configure sensor read interval 

- [] Unit tests for components
- [] Global Error Handling and logging
