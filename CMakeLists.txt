cmake_minimum_required(VERSION 3.12)

# Change your executable name to something creative!
set(NAME PICO_REST_SENSOR) # <-- Name your project/executable here!
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Pull in Pico SDK (must be before project)
include(pico_sdk_import.cmake)

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


add_subdirectory(src) # Source code dir
