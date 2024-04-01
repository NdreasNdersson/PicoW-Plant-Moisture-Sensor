#ifndef __UTILS__JSON_CONVERTER__
#define __UTILS__JSON_CONVERTER__

#include "network/wifi_config.h"
#include "nlohmann/json.hpp"
#include "sensors/sensor_config.h"

using json = nlohmann::json;

// sensor_config_t
void to_json(json &json_data, const sensor_config_t &config);
void from_json(const json &json_data, sensor_config_t &config);

// wifi_config_t
void to_json(json &json_data, const wifi_config_t &config);
void from_json(const json &json_data, wifi_config_t &config);

#endif
