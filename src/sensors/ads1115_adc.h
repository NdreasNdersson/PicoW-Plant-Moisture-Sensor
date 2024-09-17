#ifndef PICO_REST_SENSOR_SENSORS_ADS1115_ADC_H_
#define PICO_REST_SENSOR_SENSORS_ADS1115_ADC_H_

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <string>

#include "ads1115.h"
#include "hardware/i2c.h"
#include "patterns/publisher.h"
#include "patterns/subscriber.h"
#include "registers.h"
#include "sensors/sensor.h"
#include "sensors/sensor_config.h"
#include "utils/button/button_control.h"
#include "utils/low_pass_filter.h"

struct Measurement_t {
    uint16_t raw_value;
    float value;
};

class Ads1115Adc : public Sensor,
                   public Subscriber<int>,
                   public Publisher<Measurement_t> {
   public:
    Ads1115Adc(sensor_config_t &config,
               std::shared_ptr<ButtonControl> button_control,
               std::function<void(bool)> led_callback, float delta_time);
    ~Ads1115Adc();
    Ads1115Adc(const Ads1115Adc &) = default;
    Ads1115Adc(Ads1115Adc &&) = default;
    Ads1115Adc &operator=(const Ads1115Adc &) = default;
    Ads1115Adc &operator=(Ads1115Adc &&) = default;

    void init(i2c_inst_t *i2c, uint8_t address,
              const ads1115_mux_t mux_setting);
    void get_name(std::string &name) override;
    void get_config(sensor_config_t &config) override;
    auto read() -> SensorReadStatus override;
    auto get_raw_value() -> std::uint16_t override;
    auto get_value() -> float override;

    // Subscriber
    void update(const int &) override;

    // Publisher
    void attach(Subscriber<Measurement_t> *subscriber) override;
    void detach(Subscriber<Measurement_t> *subscriber) override;
    void notify(const Measurement_t &) override;

   private:
    static constexpr int SAMPLES_TO_COMPLETE_CALIBRATION{20};
    struct ads1115_adc adc_state_;
    sensor_config_t config_;
    bool calibration_run_;
    int calibration_samples_;
    std::string name_;
    std::function<void(bool)> led_callback_;
    std::uint16_t adc_value_;
    float value_;
    LowPassFilter<float> low_pass_filter_;
    std::list<Subscriber<Measurement_t> *> list_subscribers_;
    std::shared_ptr<ButtonControl> button_control_;
};

#endif  // PICO_REST_SENSOR_SENSORS_ADS1115_ADC_H_
