#ifndef PICO_REST_SENSOR_LINKER_DEFINITIONS_H
#define PICO_REST_SENSOR_LINKER_DEFINITIONS_H

#include <cstdint>

#include "hardware/flash.h"

#define XIP_BASE 0x10
#define ADDR_AS_U32(Data) (uint32_t) & (Data)
#define ADDR_WITH_XIP_OFFSET_AS_U32(Data) ADDR_AS_U32(Data) - XIP_BASE

extern uint32_t APP_STORAGE_ADDRESS;

extern uint8_t g_app_storage[FLASH_SECTOR_SIZE];

#endif  // PICO_REST_SENSOR_LINKER_DEFINITIONS_H
