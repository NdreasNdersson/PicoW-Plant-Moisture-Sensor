#include "ads1115_adc.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>

#include "sensors/sensor_config.h"
#include "utils/logging.h"

Ads1115Adc::Ads1115Adc(sensor_config_t &config,
                       std::shared_ptr<ButtonControl> button_control,
                       std::function<void(bool)> led_callback, float delta_time)
    : adc_state_{},
      config_{config},
      calibration_run_{false},
      calibration_samples_{0},
      name_(config_.type + "_" + std::to_string(config_.pin)),
      led_callback_{std::move(led_callback)},
      adc_value_{0U},
      value_{0.0F},
      low_pass_filter_(0.01f, delta_time),
      m_button_control{button_control} {
    m_button_control->attach(ButtonNames::A, this);
}

Ads1115Adc::~Ads1115Adc() { m_button_control->detach(ButtonNames::A, this); }

void Ads1115Adc::init(i2c_inst_t *i2c, uint8_t address,
                      const ads1115_mux_t mux_setting) {
    ads1115_init(i2c, address, &adc_state_);

    ads1115_set_input_mux(mux_setting, &adc_state_);
    ads1115_set_pga(ADS1115_PGA_4_096, &adc_state_);
    ads1115_set_data_rate(ADS1115_RATE_128_SPS, &adc_state_);

    ads1115_write_config(&adc_state_);
}

void Ads1115Adc::get_name(std::string &name) { name = name_; }

void Ads1115Adc::get_config(sensor_config_t &config) { config = config_; }

auto Ads1115Adc::read() -> SensorReadStatus {
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
                 static_cast<float>(config_.max_value - config_.min_value);
        if (config_.inverse_measurement) {
            value_ = (value_ - 100.0f) * (-1.0f);
        }
        value_ = low_pass_filter_.update(value_);
        led_callback_(false);
    }

    return return_status;
}

auto Ads1115Adc::get_raw_value() -> std::uint16_t { return adc_value_; }
auto Ads1115Adc::get_value() -> float { return value_; }

void Ads1115Adc::update() {
    if (!calibration_run_) {
        calibration_run_ = true;
        calibration_samples_ = 0;
        config_.max_value = std::numeric_limits<std::uint16_t>::min();
        config_.min_value = std::numeric_limits<std::uint16_t>::max();
    }
}
