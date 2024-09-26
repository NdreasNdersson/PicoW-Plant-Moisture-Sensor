#! /bin/bash
set -eux

if [ $# -eq 0 ]; then
    debug=0
    cmake --preset ninja-release
    cmake --build --preset ninja-release
else
    if [ "${1^^}" = "DEBUG" ]; then
        debug=1
    else
        debug=0
    fi
fi

if [ $debug = 1 ]; then
    cmake --preset ninja-debug
    cmake --build --preset ninja-debug
    python3 libs/PicoW-Bootloader/combine_bootloader_and_app.py \
        --bootloader-file "build/ninja-debug/libs/PicoW-Bootloader/pico_bootloader.bin" \
        --app-file "build/ninja-debug/PICO_REST_SENSOR.bin"
else
    cmake --preset ninja-release
    cmake --build --preset ninja-release
    python3 libs/PicoW-Bootloader/combine_bootloader_and_app.py \
        --bootloader-file "build/ninja-release/libs/PicoW-Bootloader/pico_bootloader.bin" \
        --app-file "build/ninja-release/PICO_REST_SENSOR.bin"
fi
