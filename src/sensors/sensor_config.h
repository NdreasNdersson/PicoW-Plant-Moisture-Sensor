#ifndef PICO_REST_SENSOR_SENSORS_SENSOR_CONFIG_H_
#define PICO_REST_SENSOR_SENSORS_SENSOR_CONFIG_H_

#include <cstdint>
#include <string>

struct sensor_config_t {
    uint8_t pin;
    std::string type;
    std::uint16_t min_value;
    std::uint16_t max_value;
    bool inverse_measurement;
};

#endif  // PICO_REST_SENSOR_SENSORS_SENSOR_CONFIG_H_
