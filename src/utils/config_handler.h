#ifndef __UTILS__CONFIG_HANDLER__
#define __UTILS__CONFIG_HANDLER__

#include <vector>

extern "C" {
#include <hardware/flash.h>
};
#include "network/wifi_config.h"
#include "nlohmann/json.hpp"
#include "sensors/sensor_config.h"

#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
#define MAX_NUMBER_OF_PAGES 4
#define MAX_FLASH_SIZE FLASH_PAGE_SIZE *MAX_NUMBER_OF_PAGES
#define MAX_VALUE_SIZE 32

class ConfigHandler {
   public:
    ConfigHandler() = default;

    using return_value_t = char[MAX_VALUE_SIZE];

    bool write_config(const std::vector<sensor_config_t> &config);
    bool write_config(const wifi_config_t &config);

    bool read_config(std::vector<sensor_config_t> &config);
    bool read_config(wifi_config_t &config);

   private:
    using pages_t = uint8_t[MAX_FLASH_SIZE];
    typedef struct {
        pages_t data;
        uint8_t number_of_pages;
    } flash_data_t;

    bool write_json_to_flash(const nlohmann::json &json_data);
    bool read_json_from_flash(nlohmann::json &json_data);
    bool write(flash_data_t &data);
    static void erase_and_program(void *data);
    void print_buf(const uint8_t *buf, size_t len);

    const uint8_t *m_flash_target_contents{
        (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET)};
};

#endif
