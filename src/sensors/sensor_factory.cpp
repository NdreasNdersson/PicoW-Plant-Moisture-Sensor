#include "sensor_factory.h"

#include "hardware/i2c.h"
#include "utils/logging.h"

SensorFactory::SensorFactory(uint8_t number_of_dacs)
    : m_adcs{}, m_number_of_dacs{number_of_dacs} {
    LogDebug(("SensorFactory ctor"));
}

std::vector<std::function<void(float &, std::string &)>> SensorFactory::create(
    std::map<int, sensor_config_t> pin_configs) {
    std::vector<std::function<void(float &, std::string &)>> return_callbacks;

    if (pin_configs.empty() || (m_number_of_dacs > MAX_NUMBER_OF_DACS) ||
        (pin_configs.end()->first >
         MAX_NUMBER_OF_ANALOG_PINS * m_number_of_dacs)) {
        LogError(("Can't initialise %u dacs", m_number_of_dacs));
        return return_callbacks;
    }

    bool status{true};
    LogDebug(("Initialise I2C"));
    // Initialise I2C
    i2c_init(I2C_PORT, I2C_FREQ);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    for (auto i{0}; i < m_number_of_dacs; i++) {
        status =
            status && m_adcs[i].init(I2C_PORT, ADS1115_I2C_FIRST_ADDRESS + i);
    }
    if (status) {
        for (auto &pin_config : pin_configs) {
            auto dac_idx{pin_config.first / MAX_NUMBER_OF_ANALOG_PINS};
            auto analog_pin_idx{pin_config.first % MAX_NUMBER_OF_ANALOG_PINS};

            LogDebug(("Create callback for ADS1115: %u analog pin: %u",
                      dac_idx + 1, analog_pin_idx));

            auto &config = pin_config.second;
            if (config.run_calibration) {
                m_adcs[dac_idx].start_calibration();
            } else {
                LogDebug(("Set max value %u, min value %u, for %s",
                          config.max_value, config.min_value,
                          config.name.c_str()));
                m_adcs[dac_idx].set_min_value(config.min_value);
                m_adcs[dac_idx].set_max_value(config.max_value);
                m_adcs[dac_idx].set_inverse_measurement(config.max_value);
                m_adcs[dac_idx].set_name(config.name);
            }

            return_callbacks.emplace_back(
                std::bind(&Ads1115Adc::read, m_adcs[dac_idx], analog_pin_idx,
                          std::placeholders::_1, std::placeholders::_2));
        }
    }

    return return_callbacks;
}
