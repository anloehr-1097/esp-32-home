# Home automation Example for the ESP32 using the ESP-IDF framework.

This repo demonstrates how the ESP-IDF framework can be used to read temperature and humidity data from a SHT3x sensor
and pushed to a dashboard running in the same local network.



# Next Steps
- [] Complete the wifi config: 
    - [] Retrieve & store credentials
    - [] Setup event group s.t. the update task is only pushing once the wifi connection is established
    - [] Setup Event handler to handle wifi events (e.g. disconnection, reconnection, etc.)

- [] Complete the update task:
    - [] Push data to the dashboard using HTTP POST requests
    - [] Retrieve the data from the queue 


- [] Global Error Handling and logging
