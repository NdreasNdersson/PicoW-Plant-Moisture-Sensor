#include "led.h"

#include <algorithm>
#include <cstring>

#include "FreeRTOS.h"
#include "pico/stdlib.h"
#include "task.h"
#include "utils/logging.h"

Led::Led(int pin_number) {
    m_task_attributes =
        std::make_unique<TaskAttributes>(pin_number, false, false, min_delay);

    gpio_init(m_task_attributes->pin_number);
    gpio_set_dir(m_task_attributes->pin_number, GPIO_OUT);
}

Led::~Led() { stop_blink(); }

void Led::set_on() {
    stop_blink();

    gpio_put(m_task_attributes->pin_number, 1);
}

void Led::set_off() {
    stop_blink();

    gpio_put(m_task_attributes->pin_number, 0);
}

void Led::set_blink_delay(std::uint16_t delay) {
    m_task_attributes->blink_delay = std::min(delay, min_delay);
}

void Led::start_blink() {
    if (!m_task_attributes->blink_task_running) {
        char thread_name[32];
        sprintf(thread_name, "BlinkThread_%d", m_task_attributes->pin_number);
        xTaskCreate(blink_task, thread_name, 128, m_task_attributes.get(),
                    LED_TASK_PRIORITY, NULL);
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
        vTaskDelay(100);
    }
}

void Led::blink_task(void *params) {
    auto *task_attributes = (TaskAttributes *)(params);

    task_attributes->blink_task_running = true;

    while (true) {
        gpio_put(task_attributes->pin_number, 1);
        vTaskDelay(task_attributes->blink_delay / portTICK_PERIOD_MS);
        gpio_put(task_attributes->pin_number, 0);
        vTaskDelay(task_attributes->blink_delay / portTICK_PERIOD_MS);
        if (task_attributes->blink_task_stop) {
            break;
        }
    }
    task_attributes->blink_task_running = false;
    task_attributes->blink_task_stop = false;

    vTaskDelete(NULL);
}
