#ifndef PICO_REST_SENSOR_NETWORK_WIFI_CONFIG_H_
#define PICO_REST_SENSOR_NETWORK_WIFI_CONFIG_H_

#include <string>

using wifi_config_t = struct wifi_config_t_ {
    std::string ssid;
    std::string password;
};

#endif  // PICO_REST_SENSOR_NETWORK_WIFI_CONFIG_H_
