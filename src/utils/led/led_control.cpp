#include "led_control.h"

void LedControl::set(LedPin pin, bool enable) {
    switch (pin) {
        case LedPin::led_a:
            if (enable) {
                m_led_a.set_on();
            } else {
                m_led_a.set_off();
            }
            break;
        case LedPin::led_b:
            if (enable) {
                m_led_b.set_on();
            } else {
                m_led_b.set_off();
            }
            break;
        case LedPin::led_c:
            if (enable) {
                m_led_c.set_on();
            } else {
                m_led_c.set_off();
            }
            break;
    }
}

void LedControl::set_blink_delay(LedPin pin, std::uint16_t delay) {
    switch (pin) {
        case LedPin::led_a:
            m_led_a.set_blink_delay(delay);
            break;
        case LedPin::led_b:
            m_led_b.set_blink_delay(delay);
            break;
        case LedPin::led_c:
            m_led_c.set_blink_delay(delay);
            break;
    }
}

void LedControl::start_blink(LedPin pin) {
    switch (pin) {
        case LedPin::led_a:
            m_led_a.start_blink();
            break;
        case LedPin::led_b:
            m_led_b.start_blink();
            break;
        case LedPin::led_c:
            m_led_c.start_blink();
            break;
    }
}

void LedControl::stop_blink(LedPin pin) {
    switch (pin) {
        case LedPin::led_a:
            m_led_a.stop_blink();
            break;
        case LedPin::led_b:
            m_led_b.stop_blink();
            break;
        case LedPin::led_c:
            m_led_c.stop_blink();
            break;
    }
}
