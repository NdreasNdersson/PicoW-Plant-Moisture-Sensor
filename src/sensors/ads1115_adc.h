#ifndef PICO_REST_SENSOR_SENSORS_ADS1115_ADC_H_
#define PICO_REST_SENSOR_SENSORS_ADS1115_ADC_H_

#include <cstdint>
#include <functional>
#include <string>

#include "patterns/subscriber.h"
#include "registers.h"
#include "sensors/sensor_config.h"
#include "utils/low_pass_filter.h"

extern "C" {
#include "ads1115.h"
}
#include "hardware/i2c.h"

enum class SensorReadStatus : uint8_t { Calibrating, CalibrationComplete, Ok };

class Ads1115Adc : public Subscriber {
   public:
    Ads1115Adc(const ads1115_mux_t mux_setting, sensor_config_t &config,
               std::function<void(bool)> led_callback, float delta_time);

    void init(i2c_inst_t *i2c, uint8_t address);
    void get_name(std::string &name);
    void get_config(sensor_config_t &config);
    auto read() -> SensorReadStatus;
    auto get_raw_value() -> std::uint16_t;
    auto get_value() -> float;
    void update() override;

   private:
    static constexpr int SAMPLES_TO_COMPLETE_CALIBRATION{20};
    struct ads1115_adc adc_state_;
    sensor_config_t config_;
    bool calibration_run_;
    int calibration_samples_;
    std::string name_;
    ads1115_mux_t mux_setting_;
    std::function<void(bool)> led_callback_;
    std::uint16_t adc_value_;
    float value_;
    LowPassFilter<float> low_pass_filter_;
};

#endif  // PICO_REST_SENSOR_SENSORS_ADS1115_ADC_H_
