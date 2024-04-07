#include "sensor_factory.h"

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "utils/button/button_control.h"
#include "utils/logging.h"

SensorFactory::SensorFactory() : m_number_of_dacs{MAX_NUMBER_OF_DACS} {}

void SensorFactory::create(const std::vector<sensor_config_t> &pin_configs,
                           std::vector<Ads1115Adc> &sensors,
                           ButtonControl &button_control) {
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

        std::string name{config.type + "_" + std::to_string(config.pin)};
        switch (analog_pin_id) {
            case 1:
                sensors.emplace_back(ADS1115_MUX_SINGLE_0, name);
                break;
            case 2:
                sensors.emplace_back(ADS1115_MUX_SINGLE_1, name);
                break;
            case 3:
                sensors.emplace_back(ADS1115_MUX_SINGLE_2, name);
                break;
            case 4:
                sensors.emplace_back(ADS1115_MUX_SINGLE_3, name);
                break;
        }

        sensors.back().init(I2C_PORT, ADS1115_I2C_FIRST_ADDRESS + dac_idx);

        button_control.attach(ButtonNames::A, &sensors.back());
        LogDebug(("Create callback for ADS1115: %u analog pin: %u", dac_id,
                  analog_pin_id));

        if ((config.max_value - config.min_value) != 0) {
            sensors.back().set_min_value(config.min_value);
            sensors.back().set_max_value(config.max_value);
        }
        sensors.back().set_inverse_measurement(config.max_value);
    }
}
