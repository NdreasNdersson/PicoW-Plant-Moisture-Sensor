#ifndef PICO_REST_SENSOR_SENSORS_SENSOR_H_
#define PICO_REST_SENSOR_SENSORS_SENSOR_H_

#include <string>

#include "sensors/sensor_config.h"

enum class SensorReadStatus : uint8_t { Calibrating, CalibrationComplete, Ok };

class Sensor {
   public:
    Sensor() = default;
    virtual ~Sensor() = default;
    Sensor(const Sensor&) = default;
    Sensor(Sensor&&) = default;
    Sensor& operator=(const Sensor&) = default;
    Sensor& operator=(Sensor&&) = default;

    virtual void get_name(std::string& name) = 0;
    virtual void get_config(sensor_config_t& config) = 0;
    virtual auto read() -> SensorReadStatus = 0;
    virtual auto get_raw_value() -> std::uint16_t = 0;
    virtual auto get_value() -> float = 0;
};

#endif  // PICO_REST_SENSOR_SENSORS_SENSOR_H_
