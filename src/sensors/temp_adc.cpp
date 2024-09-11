#include "temp_adc.h"

#include <cstdint>
#include <string>
#include <utility>

#include "hardware/adc.h"
#include "sensors/sensor_config.h"
#include "utils/logging.h"

constexpr float CONVERSION_FACTOR = 3.3f / (1 << 12);

TempAdc::TempAdc(sensor_config_t &config,
                 std::function<void(bool)> led_callback)
    : config_{config},
      name_(config_.type),
      led_callback_{std::move(led_callback)},
      adc_value_{0U},
      value_{0.0F} {}

void TempAdc::init() {
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);
}

void TempAdc::get_name(std::string &name) { name = name_; }

void TempAdc::get_config(sensor_config_t &config) { config = config_; }

auto TempAdc::read() -> SensorReadStatus {
    led_callback_(true);

    adc_value_ = adc_read();
    auto adc = static_cast<float>(adc_value_) * CONVERSION_FACTOR;
    value_ = 27.0f - (adc - 0.706f) / 0.001721f;

    led_callback_(false);

    return SensorReadStatus::Ok;
}

auto TempAdc::get_raw_value() -> std::uint16_t { return adc_value_; }
auto TempAdc::get_value() -> float { return value_; }
