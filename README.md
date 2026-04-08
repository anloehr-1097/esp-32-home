# Home automation Example for the ESP32 using the ESP-IDF framework.

This repo demonstrates how the ESP-IDF framework can be used to read temperature and humidity data from a SHT3x sensor
and pushed to a dashboard running in the same local network.



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
    - [ ] register http event handler, see docs

- [] Complete the update task:
    - [x] Push data to the dashboard using HTTP POST requests
    - [x] Retrieve the data from the queue 


- [] Unit tests for components

- [] Global Error Handling and logging
