#include "sensor_factory.h"

#include <algorithm>

#include "hardware/i2c.h"
#include "utils/logging.h"

SensorFactory::SensorFactory(uint8_t number_of_dacs)
    : m_adc_states{}, m_number_of_dacs{number_of_dacs} {
    LogDebug(("SensorFactory ctor"));
}

std::vector<std::function<float()>> SensorFactory::create(
    std::vector<int> pin_idx, bool calibrate) {
    std::vector<std::function<float()>> return_callbacks;
    auto max_pin_idx_it = max_element(std::begin(pin_idx), std::end(pin_idx));
    if ((m_number_of_dacs > MAX_NUMBER_OF_DACS) ||
        (*max_pin_idx_it > MAX_NUMBER_OF_ANALOG_PINS * m_number_of_dacs)) {
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
        status = status &&
                 m_adc_states[i].init(I2C_PORT, ADS1115_I2C_FIRST_ADDRESS + i);
        if (calibrate) {
            m_adc_states[i].start_calibration();
        }
    }
    if (status) {
        for (auto pin : pin_idx) {
            auto dac_idx{pin / MAX_NUMBER_OF_ANALOG_PINS};
            auto analog_pin_idx{pin % MAX_NUMBER_OF_ANALOG_PINS};

            LogDebug(("Create callback for ADS1115: %u analog pin: %u",
                      dac_idx + 1, analog_pin_idx));

            return_callbacks.emplace_back(std::bind(
                &Ads1115Adc::read, m_adc_states[dac_idx], analog_pin_idx));
        }
    }

    return return_callbacks;
}
