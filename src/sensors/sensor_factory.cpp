#include "sensor_factory.h"

#include "hardware/i2c.h"
#include "utils/logging.h"

SensorFactory::SensorFactory()
    : m_adcs{}, m_number_of_dacs{MAX_NUMBER_OF_DACS} {
    LogDebug(("SensorFactory ctor"));
}

std::vector<std::function<void(float &, std::string &)>> SensorFactory::create(
    std::vector<sensor_config_t> pin_configs) {
    std::vector<std::function<void(float &, std::string &)>> return_callbacks;

    if (pin_configs.empty() || (m_number_of_dacs > MAX_NUMBER_OF_DACS)) {
        LogError(("Can't initialise %u dacs", m_number_of_dacs));
        return return_callbacks;
    }

    bool status{true};
    LogDebug(("Initialise I2C"));
    // Initialise I2C
    i2c_init(I2C_PORT, I2C_FREQ);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    for (auto i{0}; i < m_number_of_dacs; i++) {
        status =
            status && m_adcs[i].init(I2C_PORT, ADS1115_I2C_FIRST_ADDRESS + i);
    }
    if (status) {
        for (auto &config : pin_configs) {
            if (config.pin < 1) {
                LogError(("Pin number can't be less than 1"));
                continue;
            }
            auto dac_idx{(config.pin - 1) / MAX_NUMBER_OF_ANALOG_PINS};
            auto dac_id{dac_idx + 1};
            auto analog_pin_idx{(config.pin - 1) % MAX_NUMBER_OF_ANALOG_PINS +
                                1};

            if (dac_id > m_number_of_dacs) {
                LogError(("DAC %u is not initialized", dac_id));
                continue;
            }

            LogDebug(("Create callback for ADS1115: %u analog pin: %u", dac_id,
                      analog_pin_idx));

            if (config.run_calibration) {
                m_adcs[dac_idx].start_calibration();
            }
            if ((config.max_value - config.min_value) != 0) {
                m_adcs[dac_idx].set_min_value(config.min_value);
                m_adcs[dac_idx].set_max_value(config.max_value);
            }
            m_adcs[dac_idx].set_inverse_measurement(config.max_value);
            std::string name{config.type + "_" + std::to_string(config.pin)};
            m_adcs[dac_idx].set_name(name);

            return_callbacks.emplace_back(
                std::bind(&Ads1115Adc::read, m_adcs[dac_idx], analog_pin_idx,
                          std::placeholders::_1, std::placeholders::_2));
        }
    }

    return return_callbacks;
}
