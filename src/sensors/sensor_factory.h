#ifndef __SENSORS__SENSOR_FACTORY__
#define __SENSORS__SENSOR_FACTORY__

#include <array>
#include <functional>
#include <string>
#include <vector>

#include "ads1115_adc.h"
#include "sensor_config.h"
#include "utils/button/button_control.h"

#define MAX_NUMBER_OF_DACS 4
#define MAX_NUMBER_OF_ANALOG_PINS 4
#define NUMBER_OF_SENSORS 5
#define I2C_PORT i2c0
#define I2C_FREQ 400000
#define ADS1115_I2C_FIRST_ADDRESS 0x48

const uint8_t SDA_PIN = 8;
const uint8_t SCL_PIN = 9;

class SensorFactory {
   public:
    SensorFactory();
    ~SensorFactory() = default;

    std::vector<std::function<void(float &, std::string &)>> create(
        const std::vector<sensor_config_t> &pin_configs,
        ButtonControl &button_control);

   private:
    std::array<Ads1115Adc, MAX_NUMBER_OF_DACS> m_adcs;
    uint8_t m_number_of_dacs;
};

#endif
