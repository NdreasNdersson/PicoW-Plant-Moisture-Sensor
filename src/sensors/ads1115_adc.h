#ifndef __SENSORS__ADS1115_ADC__
#define __SENSORS__ADS1115_ADC__

extern "C" {
#include "ads1115.h"
}
#include "hardware/i2c.h"

class Ads1115Adc {
   public:
    Ads1115Adc();
    ~Ads1115Adc() = default;

    bool init(i2c_inst_t *i2c, uint8_t address);
    void set_min_value(uint16_t value);
    void set_max_value(uint16_t value);
    float read(int pin_id);

   private:
    struct ads1115_adc m_adc_state;
    uint16_t m_min_value;
    uint16_t m_max_value;
};
#endif
