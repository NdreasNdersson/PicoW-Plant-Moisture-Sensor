#ifndef PICO_REST_SENSOR_PICO_INTERFACE_IMPL_H_
#define PICO_REST_SENSOR_PICO_INTERFACE_IMPL_H_

#include <cstddef>
#include <cstdint>

#include "pico_interface.h"

using erase_flash_t = struct erase_flash_t_ {
    uint32_t flash_offs;
    size_t count;
};
using store_to_flash_t = struct stort_to_flash_t_ {
    uint32_t flash_offs;
    const uint8_t *data;
    size_t count;
};

class PicoInterfaceImpl : public PicoInterface {
   public:
    auto store_to_flash(uint32_t flash_offs, const uint8_t *data, size_t count)
        -> bool override;
    auto erase_flash(uint32_t flash_offs, size_t count) -> bool override;

   private:
    static void program(void *data);
    static void erase(void *data);
};

#endif  // PICO_REST_SENSOR_PICO_INTERFACE_IMPL_H_
