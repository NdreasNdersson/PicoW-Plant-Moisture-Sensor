#include "sensors.h"

Sensors::Sensors(uint8_t number_of_dacs) {
    assert(number_of_dacs <= MAX_NUMBER_OF_DACS);

    for (auto i{0}; i < number_of_dacs; i++) {
        m_adc_states[i] =
            std::make_unique<Ads1115Adc>(ADS1115_I2C_FIRST_ADDRESS + i);
    }
}
