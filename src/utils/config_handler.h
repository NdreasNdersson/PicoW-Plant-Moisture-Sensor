#ifndef PICO_REST_SENSOR_UTILS_CONFIG_HANDLER_H_
#define PICO_REST_SENSOR_UTILS_CONFIG_HANDLER_H_

#include <vector>

extern "C" {
#include <hardware/flash.h>
};
#include "hal/pico_interface.h"
#include "linker_definitions.h"
#include "network/wifi_config.h"
#include "nlohmann/json.hpp"
#include "sensors/sensor_config.h"

constexpr uint8_t MAX_VALUE_SIZE{32};
constexpr uint16_t MAX_FLASH_STORAGE_SIZE{FLASH_SECTOR_SIZE};
constexpr uint8_t MAX_NUMBER_OF_PAGES{FLASH_SECTOR_SIZE / FLASH_PAGE_SIZE};

class ConfigHandler {
   public:
    ConfigHandler(PicoInterface &pico_interface);

    using return_value_t = char[MAX_VALUE_SIZE];

    auto init() -> bool;

    auto write_config(const std::vector<sensor_config_t> &config) -> bool;
    auto write_config(const wifi_config_t &config) -> bool;

    auto read_config(std::vector<sensor_config_t> &config) -> bool;
    auto read_config(wifi_config_t &config) -> bool;

   private:
    using pages_t = uint8_t[FLASH_SECTOR_SIZE];
    using flash_data_t = struct flash_data_t_ {
        pages_t data;
        size_t number_of_pages;
    };

    auto write_json_to_flash(const nlohmann::json &json_data) -> bool;
    auto read_json_from_flash(nlohmann::json &json_data) -> bool;
    auto write(flash_data_t &data) -> bool;
    static void erase_and_program(void *data);

    PicoInterface &pico_interface_;
};

#endif  // PICO_REST_SENSOR_UTILS_CONFIG_HANDLER_H_
