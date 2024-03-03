#ifndef __SENSORS__SENSOR_FACTORY__
#define __SENSORS__SENSOR_FACTORY__

#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <string>

#include "ads1115_adc.h"
#include "pico/stdlib.h"

#define MAX_NUMBER_OF_DACS 4
#define MAX_NUMBER_OF_ANALOG_PINS 4
#define NUMBER_OF_SENSORS 5
#define I2C_PORT i2c0
#define I2C_FREQ 400000
#define ADS1115_I2C_FIRST_ADDRESS 0x48

const uint8_t SDA_PIN = 8;
const uint8_t SCL_PIN = 9;

typedef struct {
    int pin;
    std::string type;
    std::uint16_t min_value;
    std::uint16_t max_value;
    bool inverse_measurement;
    bool run_calibration;
} sensor_config_t;

class SensorFactory {
   public:
    SensorFactory();
    ~SensorFactory() = default;

    std::vector<std::function<void(float &, std::string &)>> create(
        std::map<int, sensor_config_t> pin_configs);

   private:
    std::array<Ads1115Adc, MAX_NUMBER_OF_DACS> m_adcs;
    uint8_t m_number_of_dacs;
};

#endif
