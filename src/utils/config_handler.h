#ifndef __UTILS__CONFIG_HANDLER__
#define __UTILS__CONFIG_HANDLER__

extern "C" {
#include <hardware/flash.h>
};

#include "json_handler.h"

#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
#define MAX_VALUE_SIZE 32

class ConfigHandler {
    using page_t = uint8_t[FLASH_PAGE_SIZE];

   public:
    ConfigHandler(JsonHandler &json_handler);
    ~ConfigHandler() = default;

    using return_value_t = char[MAX_VALUE_SIZE];

    bool get_config_value(char const *config_name, return_value_t return_value);
    bool write_json_structure(char *json, size_t data_length);

   private:
    void read(page_t data);
    bool write(page_t data);
    void print_buf(const uint8_t *buf, size_t len);

    JsonHandler &m_json_handler;
    const uint8_t *m_flash_target_contents;
};

#endif