#ifndef __SENSORS__ADS1115_ADC__
#define __SENSORS__ADS1115_ADC__

#include <cstdint>
#include <functional>
#include <string>

#include "patterns/subscriber.h"
#include "registers.h"
#include "sensors/sensor_config.h"

extern "C" {
#include "ads1115.h"
}
#include "hardware/i2c.h"

enum class SensorReadStatus { Calibrating, CalibrationComplete, Ok };

class Ads1115Adc : public Subscriber {
   public:
    Ads1115Adc(const ads1115_mux_t mux_setting, sensor_config_t &config,
               std::function<void(bool)> led_callback);
    ~Ads1115Adc() = default;

    void init(i2c_inst_t *i2c, uint8_t address);
    void get_name(std::string &name);
    void get_config(sensor_config_t &config);
    SensorReadStatus read(float &return_value);
    void update() override;

   private:
    static constexpr int SAMPLES_TO_COMPLETE_CALIBRATION{20};
    struct ads1115_adc adc_state_;
    sensor_config_t config_;
    bool calibration_run_;
    int calibration_samples_;
    const std::string name_;
    const ads1115_mux_t mux_setting_;
    std::function<void(bool)> led_callback_;
};
#endif
