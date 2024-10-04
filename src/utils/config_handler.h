#ifndef PICO_REST_SENSOR_UTILS_CONFIG_HANDLER_H_
#define PICO_REST_SENSOR_UTILS_CONFIG_HANDLER_H_

#include <vector>

#include "network/wifi_config.h"
#include "sensors/sensor_config.h"

class ConfigHandler {
   public:
    virtual auto init() -> bool = 0;
    virtual auto write_config(const std::vector<sensor_config_t> &config)
        -> bool = 0;
    virtual auto write_config(const wifi_config_t &config) -> bool = 0;
    virtual auto read_config(std::vector<sensor_config_t> &config) -> bool = 0;
    virtual auto read_config(wifi_config_t &config) -> bool = 0;
};

#endif  // PICO_REST_SENSOR_UTILS_CONFIG_HANDLER_H_
