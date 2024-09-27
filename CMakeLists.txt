cmake_minimum_required(VERSION 3.12)

# Change your executable name to something creative!
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w) # Set the board used
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
else ()
    # Pull in Pico SDK (must be before project)
    include(${CMAKE_SOURCE_DIR}/pico_sdk_import.cmake)
endif()

project(PICO_REST_SENSOR C CXX ASM)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(unittests.cmake)
else ()
    set(NAME PICO_REST_SENSOR)

    # Initialize the Pico SDK
    pico_sdk_init()

    # Initialize all other external libs & dependencies
    add_library(LWIP_PORT INTERFACE)
    target_include_directories(LWIP_PORT
        INTERFACE
           ${CMAKE_CURRENT_LIST_DIR}/configs/lwip
        )
    SET(FREERTOS_CONFIG_FILE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/configs/FreeRTOS-Kernel" CACHE STRING "Local Config")
    include_directories("${FREERTOS_CONFIG_FILE_DIRECTORY}") # FreeRTOS config files
    include(FreeRTOS_Kernel_import.cmake) # Configure FreeRTOS Kernel
    add_subdirectory(libs/pico-ads1115/lib)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/PicoW-Bootloader)

    include(FetchContent)
    FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
    FetchContent_MakeAvailable(json)

    add_executable(${NAME}
        src/hal/pico_interface_impl.cpp
        src/utils/button/button.cpp
        src/utils/button/button_control.cpp
        src/utils/led/led_control.cpp
        src/utils/led/led.cpp
        src/utils/config_handler.cpp
        src/utils/json_converter.cpp
        src/main.cpp
        src/plant_moisture_sensor.cpp
        src/network/wifi_helper.cpp
        src/network/rest_api.cpp
        src/network/rest_api_command_handler.cpp
        src/run_time_stats/run_time_stats.cpp
        src/sensors/sensor_factory.cpp
        src/sensors/ads1115_adc.cpp
        src/sensors/temp_adc.cpp
    )

    target_include_directories(${NAME} PRIVATE src)

    target_link_options(${NAME} PRIVATE
        -L${CMAKE_CURRENT_SOURCE_DIR}/libs/PicoW-Bootloader/linker_common
    )
    pico_set_linker_script(${NAME} ${CMAKE_CURRENT_SOURCE_DIR}/libs/PicoW-Bootloader/linker_common/app.ld)

    # pull in common dependencies
    target_link_libraries(${NAME}
        pico_flash
        pico_stdlib                        # for core pico functionality
        pico_cyw43_arch_lwip_sys_freertos  # Wifi with FreeRTOS
        FreeRTOS-Kernel-Heap4              # FreeRTOS kernel and dynamic heap
        FREERTOS_PORT                      # FreeRTOS config files
        LWIP_PORT                          # LWIP config files
        pico-ads1115
        nlohmann_json::nlohmann_json
        software_download_lib
        hardware_adc
    )

    # Set logging level
    target_compile_definitions(${NAME} PRIVATE
        $<$<CONFIG:Debug>:
            DEBUG=1
        >
    )

    pico_enable_stdio_usb(${NAME} 0)
    pico_enable_stdio_uart(${NAME} 1)

    # create map/bin/hex file etc.
    pico_add_extra_outputs(${NAME})

    target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR} )
endif()

execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
)
