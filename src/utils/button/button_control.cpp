#include "button_control.h"

#include <map>

#include "FreeRTOS.h"
#include "hal/task_priorities.h"
#include "hardware/gpio.h"
#include "pico/types.h"
#include "portmacro.h"
#include "queue.h"
#include "task.h"
#include "utils/logging.h"

static std::map<pin_t, Button> buttons;
static QueueHandle_t queue = nullptr;
constexpr int BUTTON_PRESSED_VALUE{};

ButtonControl::ButtonControl() {
    queue = xQueueCreate(8, sizeof(uint));
    xTaskCreate(&ButtonControl::queue_task, "ButtonThread",
                configMINIMAL_STACK_SIZE, nullptr, BUTTON_PRESS_TASK_PRIORITY,
                nullptr);

    gpio_set_irq_callback(button_press_callback);
    for (const auto &pin : PIN_GPIOS) {
        buttons.emplace(pin.second, pin.second);
        gpio_set_irq_enabled(pin.second,
                             GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    }
}

void ButtonControl::button_press_callback(uint gpio, uint32_t event_mask) {
    LogDebug(("Button pressed 1 %u: ", gpio));
    if (event_mask == GPIO_IRQ_EDGE_RISE) {
        LogDebug(("Button pressed 2 %u: ", gpio));
        xQueueSendToBackFromISR(queue, &gpio, nullptr);
    }
}

void ButtonControl::queue_task(void * /*params*/) {
    uint gpio{};
    while (true) {
        if (queue != nullptr &&
            xQueueReceive(queue, &gpio, portMAX_DELAY) == pdTRUE) {
            auto button_it{buttons.find(gpio)};
            if (button_it != buttons.end()) {
                button_it->second.notify(BUTTON_PRESSED_VALUE);
                LogDebug(("Notify that button %u was pressed: ", gpio));
            }
        }
    }
}

void ButtonControl::attach(ButtonNames button, Subscriber<int> *subscriber) {
    buttons.at(PIN_GPIOS.at(button)).attach(subscriber);
}

void ButtonControl::detach(ButtonNames button, Subscriber<int> *subscriber) {
    buttons.at(PIN_GPIOS.at(button)).detach(subscriber);
}
