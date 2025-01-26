#ifndef PICO_REST_SENSOR_UTILS_JSON_CONVERTER_H_
#define PICO_REST_SENSOR_UTILS_JSON_CONVERTER_H_

#include "network/wifi_config.h"
#include "nlohmann/json.hpp"
#include "sensors/sensor_config.h"
#include "utils/config_handler/configs/mqtt_config.h"

using json = nlohmann::json;

// sensor_config_t
void to_json(json &json_data, const sensor_config_t &config);
void from_json(const json &json_data, sensor_config_t &config);

// wifi_config_t
void to_json(json &json_data, const wifi_config_t &config);
void from_json(const json &json_data, wifi_config_t &config);

// mqtt_config_t
void to_json(json &json_data, const mqtt_config_t &config);
void from_json(const json &json_data, mqtt_config_t &config);

#endif  // PICO_REST_SENSOR_UTILS_JSON_CONVERTER_H_
