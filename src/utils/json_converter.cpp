#include "json_converter.h"

// sensor_config_t
void to_json(json &json_data, const sensor_config_t &config) {
    json_data = json{{"pin", config.pin},
                     {"max", config.max_value},
                     {"min", config.min_value},
                     {"type", config.type},
                     {"inversed", config.inverse_measurement},
                     {"calibrate_min_value", config.calibrate_min_value},
                     {"calibrate_max_value", config.calibrate_max_value}};
}

void from_json(const json &json_data, sensor_config_t &config) {
    if (json_data.contains("pin")) {
        json_data["pin"].get_to(config.pin);
    }
    if (json_data.contains("max")) {
        json_data["max"].get_to(config.max_value);
    }
    if (json_data.contains("min")) {
        json_data["min"].get_to(config.min_value);
    }
    if (json_data.contains("type")) {
        json_data["type"].get_to(config.type);
    }
    if (json_data.contains("inversed")) {
        json_data["inversed"].get_to(config.inverse_measurement);
    }
    if (json_data.contains("calibrate_min_value")) {
        json_data["calibrate_min_value"].get_to(config.calibrate_min_value);
    }
    if (json_data.contains("calibrate_max_value")) {
        json_data["calibrate_max_value"].get_to(config.calibrate_max_value);
    }
}

// wifi_config_t
void to_json(json &json_data, const wifi_config_t &config) {
    json_data = json{{"ssid", config.ssid}, {"password", config.password}};
}

void from_json(const json &json_data, wifi_config_t &config) {
    if (json_data.contains("ssid")) {
        json_data["ssid"].get_to(config.ssid);
    }
    if (json_data.contains("password")) {
        json_data["password"].get_to(config.password);
    }
}
