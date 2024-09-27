#ifndef PICO_BOOTLOADER_PICO_INTERFACE_H_
#define PICO_BOOTLOADER_PICO_INTERFACE_H_

#include <cstddef>
#include <cstdint>

class PicoInterface {
   public:
    virtual auto store_to_flash(uint32_t flash_offs, const uint8_t *data,
                                size_t count) -> bool = 0;
    virtual auto erase_flash(uint32_t flash_offs, size_t count) -> bool = 0;
};

#endif  // PICO_BOOTLOADER_PICO_INTERFACE_H_
