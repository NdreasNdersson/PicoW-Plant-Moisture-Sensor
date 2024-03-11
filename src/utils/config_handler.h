#ifndef __UTILS__CONFIG_HANDLER__
#define __UTILS__CONFIG_HANDLER__

#include <string>

extern "C" {
#include <hardware/flash.h>
};
#include "nlohmann/json.hpp"
using json = nlohmann::json;

#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
#define MAX_VALUE_SIZE 32

class ConfigHandler {
    using page_t = uint8_t[FLASH_PAGE_SIZE];

   public:
    ConfigHandler();
    ~ConfigHandler() = default;

    using return_value_t = char[MAX_VALUE_SIZE];

    json read_json_from_flash();
    bool write_json_to_flash(std::string json);

   private:
    void read(page_t data);
    bool write(page_t data);
    static void erase_and_program(void *data);
    void print_buf(const uint8_t *buf, size_t len);

    const uint8_t *m_flash_target_contents;
};

#endif
