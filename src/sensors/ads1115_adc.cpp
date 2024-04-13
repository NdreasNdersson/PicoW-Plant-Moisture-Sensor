#include "ads1115_adc.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>

#include "sensors/sensor_config.h"
#include "utils/logging.h"

Ads1115Adc::Ads1115Adc(const ads1115_mux_t mux_setting_,
                       sensor_config_t &config,
                       std::function<void(bool)> led_callback, float delta_time)
    : adc_state_{},
      config_{config},
      calibration_run_{false},
      calibration_samples_{0},
      name_(config_.type + "_" + std::to_string(config_.pin)),
      mux_setting_(mux_setting_),
      led_callback_{led_callback},
      adc_value_{0U},
      value_{0.0F},
      low_pass_filter_(0.01f, delta_time) {}

void Ads1115Adc::init(i2c_inst_t *i2c, uint8_t address) {
    ads1115_init(i2c, address, &adc_state_);

    ads1115_set_input_mux(mux_setting_, &adc_state_);
    ads1115_set_pga(ADS1115_PGA_4_096, &adc_state_);
    ads1115_set_data_rate(ADS1115_RATE_128_SPS, &adc_state_);

    ads1115_write_config(&adc_state_);
}

void Ads1115Adc::get_name(std::string &name) { name = name_; }

void Ads1115Adc::get_config(sensor_config_t &config) { config = config_; }

SensorReadStatus Ads1115Adc::read() {
    led_callback_(true);
    auto return_status{SensorReadStatus::Ok};

    ads1115_read_adc(&adc_value_, &adc_state_);

    if (calibration_run_) {
        return_status = SensorReadStatus::Calibrating;
        config_.min_value = std::min(config_.min_value, adc_value_);
        config_.max_value = std::max(config_.max_value, adc_value_);
        calibration_samples_++;
        LogDebug(("Calibrate: sample %u, Value %u", calibration_samples_,
                  adc_value_));
        if (calibration_samples_ >= SAMPLES_TO_COMPLETE_CALIBRATION) {
            return_status = SensorReadStatus::CalibrationComplete;
            calibration_run_ = false;
            LogDebug(("New max value %u, min value %u", config_.max_value,
                      config_.min_value));
        }
    } else {
        value_ = static_cast<float>(adc_value_ - config_.min_value) * 100 /
                 (config_.max_value - config_.min_value);
        if (config_.inverse_measurement) {
            value_ = (value_ - 100.0f) * (-1.0f);
        }
        value_ = low_pass_filter_.update(value_);
        led_callback_(false);
    }

    return return_status;
}

std::uint16_t Ads1115Adc::get_raw_value() { return adc_value_; }
float Ads1115Adc::get_value() { return value_; }

void Ads1115Adc::update() {
    if (!calibration_run_) {
        calibration_run_ = true;
        calibration_samples_ = 0;
        config_.max_value = std::numeric_limits<std::uint16_t>::min();
        config_.min_value = std::numeric_limits<std::uint16_t>::max();
    }
}
