#include "ads1115_adc.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>

#include "sensors/sensor_config.h"
#include "utils/logging.h"

Ads1115Adc::Ads1115Adc(sensor_config_t &config, ButtonControl &button_control,
                       std::function<void(bool)> led_callback, float delta_time)
    : adc_state_{},
      config_{config},
      calibration_samples_{0},
      name_(config_.type + "_" + std::to_string(config_.pin)),
      led_callback_{std::move(led_callback)},
      low_pass_filter_(0.01f, delta_time),
      list_subscribers_{},
      button_control_{button_control} {}

void Ads1115Adc::init(i2c_inst_t *i2c, uint8_t address,
                      const ads1115_mux_t mux_setting) {
    ads1115_init(i2c, address, &adc_state_);

    ads1115_set_input_mux(mux_setting, &adc_state_);
    ads1115_set_pga(ADS1115_PGA_4_096, &adc_state_);
    ads1115_set_data_rate(ADS1115_RATE_128_SPS, &adc_state_);

    ads1115_write_config(&adc_state_);
}

auto Ads1115Adc::read(sensor_config_t &config) -> SensorReadStatus {
    led_callback_(true);
    auto return_status{SensorReadStatus::Ok};

    uint16_t adc_value{};
    float value{};
    ads1115_read_adc(&adc_value, &adc_state_);

    if (config_.calibrate_min_value || config_.calibrate_max_value) {
        return_status = SensorReadStatus::Calibrating;
        if (config_.calibrate_min_value) {
            if (calibration_samples_ == 0) {
                config_.min_value = std::numeric_limits<std::uint16_t>::max();
            }
            config_.min_value = std::min(config_.min_value, adc_value);
        }
        if (config_.calibrate_max_value) {
            if (calibration_samples_ == 0) {
                config_.max_value = std::numeric_limits<std::uint16_t>::min();
            }
            config_.max_value = std::max(config_.max_value, adc_value);
        }
        calibration_samples_++;
        LogDebug(("Calibrate: sample %u, Value %u", calibration_samples_,
                  adc_value));
        if (calibration_samples_ >= SAMPLES_TO_COMPLETE_CALIBRATION) {
            return_status = SensorReadStatus::CalibrationComplete;
            config_.calibrate_min_value = false;
            config_.calibrate_max_value = false;
            LogDebug(("New max value %u, min value %u", config_.max_value,
                      config_.min_value));
        }
    } else {
        value = static_cast<float>(adc_value - config_.min_value) * 100 /
                static_cast<float>(config_.max_value - config_.min_value);
        if (config_.inverse_measurement) {
            value = (value - 100.0f) * (-1.0f);
        }
        value = low_pass_filter_.update(value);
    }
    led_callback_(false);

    const Measurement_t measurement{name_, adc_value, value, config_};
    notify(measurement);

    config = config_;
    return return_status;
}

// Publisher
void Ads1115Adc::attach(Subscriber<Measurement_t> *subscriber) {
    list_subscribers_.push_back(subscriber);
}

void Ads1115Adc::detach(Subscriber<Measurement_t> *subscriber) {
    list_subscribers_.remove(subscriber);
}

void Ads1115Adc::notify(const Measurement_t &measurement) {
    for (auto subsriber : list_subscribers_) {
        subsriber->update(measurement);
    }
}
