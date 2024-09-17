#! /bin/bash
set -eux

if [ "${1^^}" = "DEBUG" ]; then
    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
else
    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
fi
cmake --build build

python3 libs/PicoW-Bootloader/combine_bootloader_and_app.py \
    --bootloader-file "build/libs/PicoW-Bootloader/bootloader/PICO_BOOTLOADER.bin" \
    --app-file "build//PICO_REST_SENSOR.bin"
