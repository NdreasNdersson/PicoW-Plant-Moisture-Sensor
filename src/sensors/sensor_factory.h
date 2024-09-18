#ifndef PICO_REST_SENSOR_SENSORS_SENSOR_FACTORY_H_
#define PICO_REST_SENSOR_SENSORS_SENSOR_FACTORY_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "sensor_config.h"
#include "sensors/sensor.h"
#include "utils/button/button_control.h"

constexpr uint8_t MAX_NUMBER_OF_DACS{4};
constexpr uint8_t MAX_NUMBER_OF_ANALOG_PINS{4};
#define I2C_PORT i2c0
#define I2C_FREQ 400000
constexpr uint8_t ADS1115_I2C_FIRST_ADDRESS{0x48};

const uint8_t SDA_PIN = 8;
const uint8_t SCL_PIN = 9;

class SensorFactory {
   public:
    void create(std::vector<sensor_config_t> &pin_configs,
                std::vector<std::shared_ptr<Sensor>> &sensors,
                ButtonControl &button_control,
                const std::function<void(bool)> &led_callback,
                float delta_time);

   private:
    uint8_t m_number_of_dacs{MAX_NUMBER_OF_DACS};
};

#endif  // PICO_REST_SENSOR_SENSORS_SENSOR_FACTORY_H_
