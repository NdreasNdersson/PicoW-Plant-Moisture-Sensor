#! /bin/bash
set -eux

cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

./libs/PicoW-Bootloader/combine_bootloader_and_app.py \
    --bootloader-file "build/libs/PicoW-Bootloader/bootloader/PICO_BOOTLOADER.bin" \
    --app-file "build//PICO_REST_SENSOR.bin"
