#include <cstdint>

#include "hardware/flash.h"
#include "linker_definitions.h"

uint8_t g_app_storage[FLASH_SECTOR_SIZE]{};
uint32_t APP_STORAGE_ADDRESS{reinterpret_cast<uint32_t>(&g_app_storage)};
