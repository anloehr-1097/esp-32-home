idf_component_register(SRCS "hello_world_main.cpp" "TempTask.cpp" "Sht3x.cpp" "UpdateTask.cpp"
                       PRIV_REQUIRES spi_flash
                       REQUIRES esp_driver_gpio esp_http_client
                       INCLUDE_DIRS "" "include")
