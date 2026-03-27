idf_component_register(SRCS "hello_world_main.cpp" "TempTask.cpp" "Sht3x.cpp" "UpdateTask.cpp"
                       PRIV_REQUIRES spi_flash
                       REQUIRES esp_driver_gpio esp_http_client fmt
                        INCLUDE_DIRS "" "include")

target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++20)
component_compile_options(-Wno-error=missing-field-initializers)
