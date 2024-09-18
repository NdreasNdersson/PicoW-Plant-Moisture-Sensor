#include "temp_adc.h"

#include <string>
#include <utility>

#include "hardware/adc.h"
#include "sensors/sensor_config.h"
#include "utils/logging.h"

constexpr float CONVERSION_FACTOR = 3.3f / (1 << 12);

TempAdc::TempAdc(sensor_config_t &config,
                 std::function<void(bool)> led_callback)
    : name_(config.type),
      led_callback_{std::move(led_callback)},
      list_subscribers_{} {}

void TempAdc::init() {
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);
}

auto TempAdc::read(sensor_config_t &config) -> SensorReadStatus {
    led_callback_(true);

    auto adc_value = adc_read();
    auto adc = static_cast<float>(adc_value) * CONVERSION_FACTOR;
    auto value = 27.0f - (adc - 0.706f) / 0.001721f;

    led_callback_(false);

    const Measurement_t measurement{name_, adc_value, value, config_};
    notify(measurement);

    config = sensor_config_t{};
    return SensorReadStatus::Ok;
}

// Publisher
void TempAdc::attach(Subscriber<Measurement_t> *subscriber) {
    list_subscribers_.push_back(subscriber);
}

void TempAdc::detach(Subscriber<Measurement_t> *subscriber) {
    list_subscribers_.remove(subscriber);
}

void TempAdc::notify(const Measurement_t &measurement) {
    for (auto subsriber : list_subscribers_) {
        subsriber->update(measurement);
    }
}
