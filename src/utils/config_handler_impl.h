#ifndef PICO_REST_SENSOR_UTILS_CONFIG_HANDLER_IMPL_H_
#define PICO_REST_SENSOR_UTILS_CONFIG_HANDLER_IMPL_H_

#include <memory>
#include <vector>

#include "utils/config_handler.h"

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

class ConfigHandlerImpl : public ConfigHandler {
   public:
    ConfigHandlerImpl(PicoInterface &pico_interface);

    auto init() -> bool override;

    auto write_config(const std::vector<sensor_config_t> &config)
        -> bool override;
    auto write_config(const wifi_config_t &config) -> bool override;

    auto read_config(std::vector<sensor_config_t> &config) -> bool override;
    auto read_config(wifi_config_t &config) -> bool override;

   private:
    auto write_json_to_flash(const nlohmann::json &json_data) -> bool;
    auto read_json_from_flash(nlohmann::json &json_data) -> bool;

    PicoInterface &pico_interface_;
};

#endif  // PICO_REST_SENSOR_UTILS_CONFIG_HANDLER_IMPL_H_
