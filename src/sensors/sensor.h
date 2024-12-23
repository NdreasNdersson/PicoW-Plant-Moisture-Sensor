#ifndef PICO_REST_SENSOR_SENSORS_SENSOR_H_
#define PICO_REST_SENSOR_SENSORS_SENSOR_H_

#include <string>

#include "sensor_config.h"
#include "src/patterns/publisher.h"

enum class SensorReadStatus : uint8_t { Calibrating, CalibrationComplete, Ok };

struct Measurement_t {
    std::string name;
    uint16_t raw_value;
    float value;
    sensor_config_t config;
};

class Sensor : public Publisher<Measurement_t> {
   public:
    Sensor() = default;
    virtual ~Sensor() = default;
    Sensor(const Sensor&) = default;
    Sensor(Sensor&&) = default;
    Sensor& operator=(const Sensor&) = default;
    Sensor& operator=(Sensor&&) = default;

    virtual auto read(sensor_config_t& config) -> SensorReadStatus = 0;

    // Publisher
    virtual void attach(Subscriber<Measurement_t>* subscriber) override = 0;
    virtual void detach(Subscriber<Measurement_t>* subscriber) override = 0;
    virtual void notify(const Measurement_t&) override = 0;
};

#endif  // PICO_REST_SENSOR_SENSORS_SENSOR_H_
