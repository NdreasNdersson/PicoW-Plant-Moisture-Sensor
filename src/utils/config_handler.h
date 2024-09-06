#ifndef __UTILS__CONFIG_HANDLER__
#define __UTILS__CONFIG_HANDLER__

#include <vector>

extern "C" {
#include <hardware/flash.h>
};
#include "linker_definitions.h"
#include "network/wifi_config.h"
#include "nlohmann/json.hpp"
#include "sensors/sensor_config.h"

enum { MAX_NUMBER_OF_PAGES = 4 };
#define MAX_FLASH_SIZE FLASH_PAGE_SIZE *MAX_NUMBER_OF_PAGES
enum { MAX_VALUE_SIZE = 32 };

class ConfigHandler {
   public:
    ConfigHandler();

    using return_value_t = char[MAX_VALUE_SIZE];

    auto write_config(const std::vector<sensor_config_t> &config) -> bool;
    auto write_config(const wifi_config_t &config) -> bool;

    auto read_config(std::vector<sensor_config_t> &config) -> bool;
    auto read_config(wifi_config_t &config) -> bool;

   private:
    using pages_t = uint8_t[MAX_FLASH_SIZE];
    using flash_data_t = struct flash_data_t_ {
        pages_t data;
        uint8_t number_of_pages;
    };

    auto write_json_to_flash(const nlohmann::json &json_data) -> bool;
    auto read_json_from_flash(nlohmann::json &json_data) -> bool;
    auto write(flash_data_t &data) -> bool;
    static void erase_and_program(void *data);
    void print_buf(const uint8_t *buf, size_t len);

    const uint8_t *m_flash_target_contents;
};

#endif
