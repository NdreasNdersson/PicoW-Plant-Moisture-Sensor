#ifndef __SENSORS__SENSOR_FACTORY__
#define __SENSORS__SENSOR_FACTORY__

#include <functional>
#include <vector>

#include "sensor_config.h"
#include "sensors/ads1115_adc.h"
#include "utils/button/button_control.h"

enum { MAX_NUMBER_OF_DACS = 4, MAX_NUMBER_OF_ANALOG_PINS = 4 };
#define I2C_PORT i2c0
enum { I2C_FREQ = 400000, ADS1115_I2C_FIRST_ADDRESS = 0x48 };

const uint8_t SDA_PIN = 8;
const uint8_t SCL_PIN = 9;

class SensorFactory {
   public:
    SensorFactory();
    ~SensorFactory() = default;

    void create(std::vector<sensor_config_t> &pin_configs,
                std::vector<Ads1115Adc> &sensors, ButtonControl &button_control,
                std::function<void(bool)> led_callback, float delta_time);

   private:
    uint8_t m_number_of_dacs{MAX_NUMBER_OF_DACS};
};

#endif
