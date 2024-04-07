#include "ads1115_adc.h"

#include <algorithm>
#include <limits>

#include "utils/logging.h"

Ads1115Adc::Ads1115Adc(const ads1115_mux_t mux_setting_,
                       const std::string &name)
    : adc_state_{},
      min_value_{std::numeric_limits<std::uint16_t>::min()},
      max_value_{std::numeric_limits<std::uint16_t>::max()},
      inverse_measurement_{false},
      calibration_complete_{false},
      calibration_run_{false},
      calibration_samples_{0},
      name_(name),
      mux_setting_(mux_setting_) {}

void Ads1115Adc::init(i2c_inst_t *i2c, uint8_t address) {
    LogDebug(("Initialise ads1115"));
    ads1115_init(i2c, address, &adc_state_);

    ads1115_set_input_mux(mux_setting_, &adc_state_);
    ads1115_set_pga(ADS1115_PGA_4_096, &adc_state_);
    ads1115_set_data_rate(ADS1115_RATE_128_SPS, &adc_state_);

    ads1115_write_config(&adc_state_);
}

void Ads1115Adc::set_min_value(const std::uint16_t value) {
    min_value_ = value;
}
void Ads1115Adc::set_max_value(const std::uint16_t value) {
    max_value_ = value;
}
void Ads1115Adc::set_inverse_measurement(const bool inverse_measurement) {
    inverse_measurement_ = inverse_measurement;
}

void Ads1115Adc::get_name(std::string &name) { name = name_; }

void Ads1115Adc::read(float &return_value) {
    return_value = 0.0f;
    std::uint16_t adc_value;

    ads1115_read_adc(&adc_value, &adc_state_);

    if (calibration_run_ && !calibration_complete_) {
        min_value_ = std::min(min_value_, adc_value);
        max_value_ = std::max(max_value_, adc_value);
        calibration_samples_++;
        LogDebug(("Calibrate: sample %u, Value %u", calibration_samples_,
                  adc_value));
        if (calibration_samples_ >= SAMPLES_TO_COMPLETE_CALIBRATION) {
            calibration_complete_ = true;
            calibration_run_ = false;
            LogDebug(
                ("New max value %u, min value %u", max_value_, min_value_));
        }
    } else {
        return_value = static_cast<float>(adc_value - min_value_) * 100 /
                       (max_value_ - min_value_);
        if (inverse_measurement_) {
            return_value = (return_value - 100.0f) * (-1.0f);
        }
    }
}

void Ads1115Adc::update() {
    if (!calibration_run_) {
        calibration_run_ = true;
        calibration_complete_ = false;
        calibration_samples_ = 0;
        max_value_ = std::numeric_limits<std::uint16_t>::min();
        min_value_ = std::numeric_limits<std::uint16_t>::max();
    }
}
