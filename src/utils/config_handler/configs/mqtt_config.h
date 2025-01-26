#ifndef PICO_REST_SENSOR_UTILS_CONFIG_HANDLER_CONFIGS_MQTT_CONFIG
#define PICO_REST_SENSOR_UTILS_CONFIG_HANDLER_CONFIGS_MQTT_CONFIG

#include <cstdint>
#include <string>

using mqtt_config_t = struct mqtt_config_t_ {
    std::string user;
    std::string password;
    std::string ip;
    std::uint16_t port;
    std::string client_name;
};

#endif  // PICO_REST_SENSOR_UTILS_CONFIG_HANDLER_CONFIGS_MQTT_CONFIG
