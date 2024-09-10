#include "led.h"

#include <algorithm>
#include <string>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "hardware/gpio.h"
#include "pico/types.h"
#include "portmacro.h"
#include "task.h"
#include "utils/logging.h"

Led::Led(uint pin_number) {
    m_task_attributes =
        std::make_unique<TaskAttributes>(pin_number, false, false, min_delay);

    gpio_init(m_task_attributes->pin_number);
    gpio_set_dir(m_task_attributes->pin_number, GPIO_OUT);
}

Led::~Led() {
    stop_blink();
    m_task_attributes.reset();
}

void Led::set_on() {
    stop_blink();

    gpio_put(m_task_attributes->pin_number, true);
}

void Led::set_off() {
    stop_blink();

    gpio_put(m_task_attributes->pin_number, false);
}

void Led::set_blink_delay(std::uint16_t delay) {
    m_task_attributes->blink_delay = std::min(delay, min_delay);
}

void Led::start_blink() {
    if (!m_task_attributes->blink_task_running) {
        const auto thread_name{"BlinkThread_" +
                               std::to_string(m_task_attributes->pin_number)};
        xTaskCreate(blink_task, thread_name.c_str(), configMINIMAL_STACK_SIZE,
                    m_task_attributes.get(), LED_TASK_PRIORITY, nullptr);
    }
}

void Led::stop_blink() {
    if (m_task_attributes->blink_task_running) {
        m_task_attributes->blink_task_stop = true;
    }
    while (true) {
        if (!m_task_attributes->blink_task_running) {
            break;
        }
        const TickType_t sleep_time{100};
        vTaskDelay(sleep_time);
    }
}

void Led::blink_task(void *params) {
    auto *task_attributes = static_cast<TaskAttributes *>(params);

    task_attributes->blink_task_running = true;

    while (true) {
        gpio_put(task_attributes->pin_number, true);
        vTaskDelay(task_attributes->blink_delay / portTICK_PERIOD_MS);
        gpio_put(task_attributes->pin_number, false);
        vTaskDelay(task_attributes->blink_delay / portTICK_PERIOD_MS);
        if (task_attributes->blink_task_stop) {
            break;
        }
    }
    task_attributes->blink_task_running = false;
    task_attributes->blink_task_stop = false;

    vTaskDelete(nullptr);
}
