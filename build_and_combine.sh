#! /bin/bash
set -eux

cmake -B build -G Ninja
cmake --build build

./external/PicoW-Bootloader/combine_bootloader_and_app.py \
    --bootloader-file "build/external/PicoW-Bootloader/bootloader/PICO_BOOTLOADER.bin" \
    --app-file "build//PICO_REST_SENSOR.bin"
