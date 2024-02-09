#ifndef __SENSORS__SENSORS__
#define __SENSORS__SENSORS__

#include <array>
#include <memory>

#include "ads1115_adc.h"

#define MAX_NUMBER_OF_DACS 4
#define NUMBER_OF_SENSORS 5
#define I2C_PORT i2c0
#define I2C_FREQ 400000
#define ADS1115_I2C_FIRST_ADDRESS 0x48

const uint8_t SDA_PIN = 8;
const uint8_t SCL_PIN = 9;

class Sensors {
   public:
    Sensors(uint8_t number_of_dacs);
    ~Sensors() = default;

   private:
    std::array<std::unique_ptr<Ads1115Adc>, MAX_NUMBER_OF_DACS> m_adc_states;
};

#endif
