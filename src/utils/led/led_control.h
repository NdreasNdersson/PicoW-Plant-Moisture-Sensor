#ifndef PICO_REST_SENSOR_UTILS_LED_LED_CONTROL_H_
#define PICO_REST_SENSOR_UTILS_LED_LED_CONTROL_H_

#include <cstdint>

#include "hal/pin_map.h"
#include "led.h"

enum class LedPin : uint8_t { led_a, led_b, led_c };

class LedControl {
   public:
    LedControl() = default;

    void set(LedPin pin, bool enable);
    void set_blink_delay(LedPin pin, std::uint16_t delay);
    void start_blink(LedPin pin);
    void stop_blink(LedPin pin);

   private:
    Led m_led_a{LED_PIN_A};
    Led m_led_b{LED_PIN_B};
    Led m_led_c{LED_PIN_C};
};

#endif  // PICO_REST_SENSOR_UTILS_LED_LED_CONTROL_H_
