#ifndef PICO_REST_SENSOR_UTILS_LED_LED_H_
#define PICO_REST_SENSOR_UTILS_LED_LED_H_

#include <atomic>
#include <cstdint>
#include <memory>

#include "pico/types.h"

#define LED_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)

class Led {
   public:
    explicit Led(uint pin_number);
    ~Led();
    Led(const Led&) = default;
    Led(Led&&) = default;
    Led& operator=(const Led&) = default;
    Led& operator=(Led&&) = default;

    void set_on();
    void set_off();
    void set_blink_delay(std::uint16_t delay);
    void start_blink();
    void stop_blink();

   private:
    class TaskAttributes {
       public:
        TaskAttributes(int pin_number, bool blink_task_stop,
                       bool blink_task_running, std::uint16_t blink_delay)
            : pin_number(pin_number),
              blink_task_stop(blink_task_stop),
              blink_task_running(blink_task_running),
              blink_delay(blink_delay) {}
        const int pin_number;
        std::atomic<bool> blink_task_stop;
        std::atomic<bool> blink_task_running;
        std::atomic<std::uint16_t> blink_delay;
    };

    static void blink_task(void* params);

    static constexpr std::uint16_t min_delay{100};

    std::unique_ptr<TaskAttributes> m_task_attributes;
};

#endif  // PICO_REST_SENSOR_UTILS_LED_LED_H_
