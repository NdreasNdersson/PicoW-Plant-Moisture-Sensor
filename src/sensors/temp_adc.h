#ifndef PICO_REST_SENSOR_SENSORS_TEMP_ADC_H_
#define PICO_REST_SENSOR_SENSORS_TEMP_ADC_H_

#include <cstdint>
#include <functional>
#include <string>

#include "sensors/sensor.h"
#include "sensors/sensor_config.h"

class TempAdc : public Sensor {
   public:
    TempAdc(sensor_config_t &config, std::function<void(bool)> led_callback);

    void init();
    void get_name(std::string &name) override;
    void get_config(sensor_config_t &config) override;
    auto read() -> SensorReadStatus override;
    auto get_raw_value() -> std::uint16_t override;
    auto get_value() -> float override;

   private:
    sensor_config_t config_;
    std::string name_;
    std::function<void(bool)> led_callback_;
    std::uint16_t adc_value_;
    float value_;
};

#endif  // PICO_REST_SENSOR_SENSORS_TEMP_ADC_H_
