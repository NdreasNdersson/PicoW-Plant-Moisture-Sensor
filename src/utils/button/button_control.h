#ifndef PICO_REST_SENSOR_UTILS_BUTTON_BUTTON_CONTROL_H_
#define PICO_REST_SENSOR_UTILS_BUTTON_BUTTON_CONTROL_H_

#include <cstdint>
#include <map>

#include "utils/button/button.h"

enum class ButtonNames : uint8_t { A, B, C };
static const std::map<ButtonNames, pin_t> PIN_GPIOS{
    {ButtonNames::C, 0}, {ButtonNames::B, 2}, {ButtonNames::A, 4}};

class ButtonControl {
   public:
    ButtonControl();

    void attach(ButtonNames button, Subscriber<int> *subscriber);
    void detach(ButtonNames button, Subscriber<int> *subscriber);

   private:
    static void button_press_callback(uint gpio, uint32_t event_mask);
    static void queue_task(void *params);
    static void enable_irq(bool state, uint gpio);
};

#endif  // PICO_REST_SENSOR_UTILS_BUTTON_BUTTON_CONTROL_H_
