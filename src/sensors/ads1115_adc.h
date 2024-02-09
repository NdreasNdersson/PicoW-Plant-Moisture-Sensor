#ifndef __SENSORS__ADS1115_ADC__
#define __SENSORS__ADS1115_ADC__

extern "C" {
#include "ads1115.h"
}

class Ads1115Adc {
   public:
    Ads1115Adc(uint8_t address);
    ~Ads1115Adc() = default;

   private:
    struct ads1115_adc m_adc_state;
    uint8_t m_address;
};
#endif
