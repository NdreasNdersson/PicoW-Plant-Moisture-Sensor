#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

export FREERTOS_KERNEL_PATH=${SCRIPT_DIR}/external/FreeRTOS-Kernel
export PICO_SDK_PATH=${SCRIPT_DIR}/external/pico-sdk
export CMAKE_GENERATOR=Ninja
