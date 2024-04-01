#ifndef __UTILS__LED__LED_CONTROL__
#define __UTILS__LED__LED_CONTROL__

#include "led.h"

enum class LedPin { led_a, led_b, led_c };

class LedControl {
   public:
    LedControl();
    ~LedControl() = default;

    void set(LedPin pin, bool enable);
    void set_blink_delay(LedPin pin, std::uint16_t delay);
    void start_blink(LedPin pin);
    void stop_blink(LedPin pin);

   private:
    Led m_led_a;
    Led m_led_b;
    Led m_led_c;
};

#endif
