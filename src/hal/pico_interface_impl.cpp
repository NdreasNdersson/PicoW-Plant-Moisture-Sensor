#include "pico_interface_impl.h"

#include <cstdio>

#include "hardware/flash.h"
#include "pico/error.h"
#include "pico/flash.h"

constexpr uint32_t ENTER_EXIT_TIMEOUT_MS{1000U};

auto PicoInterfaceImpl::store_to_flash(uint32_t flash_offs, const uint8_t *data,
                                       size_t count) -> bool {
    store_to_flash_t store_data{flash_offs, data, count};
    return PICO_OK == flash_safe_execute(&program,
                                         reinterpret_cast<void *>(&store_data),
                                         ENTER_EXIT_TIMEOUT_MS);
}

auto PicoInterfaceImpl::erase_flash(uint32_t flash_offs, size_t count) -> bool {
    erase_flash_t erase_data{flash_offs, count};
    return PICO_OK == flash_safe_execute(&erase,
                                         reinterpret_cast<void *>(&erase_data),
                                         ENTER_EXIT_TIMEOUT_MS);
}

void PicoInterfaceImpl::program(void *data) {
    auto store_to_flash_data{reinterpret_cast<store_to_flash_t *>(data)};
    flash_range_program(store_to_flash_data->flash_offs,
                        store_to_flash_data->data, store_to_flash_data->count);
}

void PicoInterfaceImpl::erase(void *data) {
    auto erase_data{reinterpret_cast<erase_flash_t *>(data)};
    flash_range_erase(erase_data->flash_offs, erase_data->count);
}
