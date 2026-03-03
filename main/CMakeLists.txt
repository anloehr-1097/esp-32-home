idf_component_register(SRCS "hello_world_main.cpp" "TempTask.cpp" "Sht3x.cpp"
                       PRIV_REQUIRES spi_flash
                       REQUIRES esp_driver_gpio
                       INCLUDE_DIRS "" "include")
