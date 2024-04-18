#include "sensor_factory.h"

#include <functional>
#include <limits>

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "utils/button/button_control.h"
#include "utils/logging.h"

SensorFactory::SensorFactory() = default;

void SensorFactory::create(std::vector<sensor_config_t> &pin_configs,
                           std::vector<Ads1115Adc> &sensors,
                           ButtonControl &button_control,
                           std::function<void(bool)> led_callback,
                           float delta_time) {
    if (pin_configs.empty() || (m_number_of_dacs > MAX_NUMBER_OF_DACS)) {
        LogError(("Can't initialise %u dacs", m_number_of_dacs));
        return;
    }

    bool status{true};
    i2c_init(I2C_PORT, I2C_FREQ);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    sensors.reserve(pin_configs.size());
    for (auto &config : pin_configs) {
        if (config.pin < 1) {
            LogError(("Pin number can't be less than 1"));
            continue;
        }
        auto dac_idx{(config.pin - 1) / MAX_NUMBER_OF_ANALOG_PINS};
        auto dac_id{dac_idx + 1};
        auto analog_pin_id{(config.pin - 1) % MAX_NUMBER_OF_ANALOG_PINS + 1};

        if (dac_id > m_number_of_dacs) {
            LogError(("DAC %u are not in use", dac_id));
            continue;
        }

        if ((config.max_value - config.min_value) == 0) {
            config.min_value = std::numeric_limits<std::uint16_t>::min();
            config.max_value = std::numeric_limits<std::uint16_t>::max();
        }

        switch (analog_pin_id) {
            case 1:
                sensors.emplace_back(ADS1115_MUX_SINGLE_0, config, led_callback,
                                     delta_time);
                break;
            case 2:
                sensors.emplace_back(ADS1115_MUX_SINGLE_1, config, led_callback,
                                     delta_time);
                break;
            case 3:
                sensors.emplace_back(ADS1115_MUX_SINGLE_2, config, led_callback,
                                     delta_time);
                break;
            case 4:
                sensors.emplace_back(ADS1115_MUX_SINGLE_3, config, led_callback,
                                     delta_time);
                break;
        }

        LogDebug(("Init ADS1115: %u analog pin: %u", dac_id, analog_pin_id));
        sensors.back().init(I2C_PORT, ADS1115_I2C_FIRST_ADDRESS + dac_idx);

        button_control.attach(ButtonNames::A, &sensors.back());
    }
}
