#ifndef PICO_REST_SENSOR_SENSORS_TEMP_ADC_H_
#define PICO_REST_SENSOR_SENSORS_TEMP_ADC_H_

#include <cstdint>
#include <functional>
#include <list>
#include <string>

#include "sensors/sensor.h"
#include "sensors/sensor_config.h"

class TempAdc : public Sensor {
   public:
    TempAdc(sensor_config_t &config, std::function<void(bool)> led_callback);

    void init();
    auto read(sensor_config_t &config) -> SensorReadStatus override;

    // Publisher
    void attach(Subscriber<Measurement_t> *subscriber) override;
    void detach(Subscriber<Measurement_t> *subscriber) override;
    void notify(const Measurement_t &) override;

   private:
    sensor_config_t config_;
    std::string name_;
    std::function<void(bool)> led_callback_;
    std::list<Subscriber<Measurement_t> *> list_subscribers_;
};

#endif  // PICO_REST_SENSOR_SENSORS_TEMP_ADC_H_
