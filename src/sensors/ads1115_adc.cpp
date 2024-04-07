#include "ads1115_adc.h"

#include <algorithm>
#include <limits>
#include <string>

#include "sensors/sensor_config.h"
#include "utils/logging.h"

Ads1115Adc::Ads1115Adc(const ads1115_mux_t mux_setting_,
                       sensor_config_t &config,
                       std::function<void(bool)> led_callback)
    : adc_state_{},
      config_{config},
      calibration_run_{false},
      calibration_samples_{0},
      name_(config_.type + "_" + std::to_string(config_.pin)),
      mux_setting_(mux_setting_),
      led_callback_{led_callback} {}

void Ads1115Adc::init(i2c_inst_t *i2c, uint8_t address) {
    ads1115_init(i2c, address, &adc_state_);

    ads1115_set_input_mux(mux_setting_, &adc_state_);
    ads1115_set_pga(ADS1115_PGA_4_096, &adc_state_);
    ads1115_set_data_rate(ADS1115_RATE_128_SPS, &adc_state_);

    ads1115_write_config(&adc_state_);
}

void Ads1115Adc::get_name(std::string &name) { name = name_; }

void Ads1115Adc::get_config(sensor_config_t &config) { config = config_; }

SensorReadStatus Ads1115Adc::read(float &return_value) {
    led_callback_(true);
    auto return_status{SensorReadStatus::Ok};
    return_value = 0.0f;
    std::uint16_t adc_value;

    ads1115_read_adc(&adc_value, &adc_state_);

    if (calibration_run_) {
        return_status = SensorReadStatus::Calibrating;
        config_.min_value = std::min(config_.min_value, adc_value);
        config_.max_value = std::max(config_.max_value, adc_value);
        calibration_samples_++;
        LogDebug(("Calibrate: sample %u, Value %u", calibration_samples_,
                  adc_value));
        if (calibration_samples_ >= SAMPLES_TO_COMPLETE_CALIBRATION) {
            return_status = SensorReadStatus::CalibrationComplete;
            calibration_run_ = false;
            LogDebug(("New max value %u, min value %u", config_.max_value,
                      config_.min_value));
        }
    } else {
        return_value = static_cast<float>(adc_value - config_.min_value) * 100 /
                       (config_.max_value - config_.min_value);
        if (config_.inverse_measurement) {
            return_value = (return_value - 100.0f) * (-1.0f);
        }
        led_callback_(false);
    }

    return return_status;
}

void Ads1115Adc::update() {
    if (!calibration_run_) {
        calibration_run_ = true;
        calibration_samples_ = 0;
        config_.max_value = std::numeric_limits<std::uint16_t>::min();
        config_.min_value = std::numeric_limits<std::uint16_t>::max();
    }
}
