cmake_minimum_required(VERSION 3.12)

# Change your executable name to something creative!
set(NAME PICO_REST_SENSOR) # <-- Name your project/executable here!
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Pull in Pico SDK (must be before project)
include(external/pico-sdk/pico_sdk_init.cmake)

project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(PICO_BOARD pico_w) # Set the board used

# Set config files paths to be used
include(lwip_import.cmake) # lwip config files
SET(FREERTOS_CONFIG_FILE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/configs/FreeRTOS-Kernel" CACHE STRING "Local Config")
include_directories("${FREERTOS_CONFIG_FILE_DIRECTORY}") # FreeRTOS config files

# Initialize the Pico SDK
pico_sdk_init()

# Initialize all other external libs & dependencies
include(FreeRTOS_Kernel_import.cmake) # Configure FreeRTOS Kernel
include(pico_ads1115_import.cmake)

include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

add_executable(${NAME}
    src/utils/button/button.cpp
    src/utils/button/button_control.cpp
    src/utils/led/led_control.cpp
    src/utils/led/led.cpp
    src/utils/logger.cpp
    src/utils/config_handler.cpp
    src/utils/json_converter.cpp
    src/main.cpp
    src/network/wifi_helper.cpp
    src/network/rest_api.cpp
    src/run_time_stats/run_time_stats.cpp
    src/sensors/sensor_factory.cpp
    src/sensors/ads1115_adc.cpp
)

target_include_directories(${NAME} PRIVATE src)

# Add bootloader
add_subdirectory(${CMAKE_SOURCE_DIR}/external/PicoW-Bootloader)
target_link_options(${NAME} PRIVATE
    -L${CMAKE_CURRENT_SOURCE_DIR}/external/PicoW-Bootloader/linker_common
)
pico_set_linker_script(${NAME} ${CMAKE_CURRENT_SOURCE_DIR}/external/PicoW-Bootloader/linker_common/app.ld)

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
    PICO_BOOTLOADER_LIB
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
