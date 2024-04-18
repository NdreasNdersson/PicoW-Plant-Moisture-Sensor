#ifndef __SENSORS__SENSOR_CONFIG__
#define __SENSORS__SENSOR_CONFIG__

#include <cstdint>
#include <string>

using sensor_config_t = struct sensor_config {
    int pin;
    std::string type;
    std::uint16_t min_value;
    std::uint16_t max_value;
    bool inverse_measurement;
};

#endif
