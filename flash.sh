#!/usr/bin/env bash

pushd build
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program PICO_REST_SENSOR.elf verify reset exit"
popd
