#ifndef __SENSORS__ADS1115_ADC__
#define __SENSORS__ADS1115_ADC__

#include <cstdint>
#include <string>

#include "patterns/subscriber.h"
#include "registers.h"

extern "C" {
#include "ads1115.h"
}
#include "hardware/i2c.h"

class Ads1115Adc : public Subscriber {
   public:
    Ads1115Adc(const ads1115_mux_t mux_setting, const std::string &name);
    ~Ads1115Adc() = default;

    void init(i2c_inst_t *i2c, uint8_t address);
    void set_min_value(const std::uint16_t value);
    void set_max_value(const std::uint16_t value);
    void set_inverse_measurement(const bool inverse);
    void get_name(std::string &name);
    void read(float &return_value);
    void update() override;

   private:
    static constexpr int SAMPLES_TO_COMPLETE_CALIBRATION{20};
    struct ads1115_adc adc_state_;
    std::uint16_t min_value_;
    std::uint16_t max_value_;
    bool inverse_measurement_;
    bool calibration_run_;
    bool calibration_complete_;
    int calibration_samples_;
    const std::string name_;
    const ads1115_mux_t mux_setting_;
};
#endif
