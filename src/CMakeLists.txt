add_executable(${NAME}
        utils/led/led_control.cpp
        utils/led/led.cpp
        utils/logger.cpp
        utils/config_handler.cpp
        main.cpp
        network/wifi_helper.cpp
        network/rest_api.cpp
        network/tcp_server.c
        run_time_stats/run_time_stats.cpp
        sensors/sensor_factory.cpp
        sensors/ads1115_adc.cpp
        )

# pull in common dependencies
target_link_libraries(${NAME}
        pico_stdlib                        # for core pico functionality
        pico_cyw43_arch_lwip_sys_freertos  # Wifi with FreeRTOS
        FreeRTOS-Kernel-Heap4              # FreeRTOS kernel and dynamic heap
        FREERTOS_PORT                      # FreeRTOS config files
        LWIP_PORT                          # LWIP config files
        pico-ads1115
        nlohmann_json::nlohmann_json
        )

# Set logging level
target_compile_definitions(${NAME} PRIVATE
    $<$<CONFIG:Debug>:
        DEBUG=1
    >
)

pico_enable_stdio_usb(${NAME} 1)
pico_enable_stdio_uart(${NAME} 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})

target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR} )
