#include <cstdio>

#include "FreeRTOS.h"
#include "common_definitions.h"
#include "hal/task_priorities.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "run_time_stats/run_time_stats.h"
#include "src/plant_moisture_sensor.h"
#include "task.h"
#include "utils/logging.h"

#define PRINT_TASK_INFO (0)

void status_task(void *) {
    while (true) {
        runTimeStats();

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void main_task(void *) {
    auto plant_moisture_sensor{PlantMoistureSensor()};
    plant_moisture_sensor.init();
    plant_moisture_sensor.loop();
}

void vLaunch() {
    xTaskCreate(main_task, "MainThread", 2048, nullptr, MAIN_TASK_PRIORITY,
                nullptr);
#if PRINT_TASK_INFO == 1
    xTaskCreate(status_task, "StatusThread", configMINIMAL_STACK_SIZE, nullptr,
                STATUS_TASK_PRIORITY, nullptr);
#endif

    vTaskStartScheduler();
}

auto main() -> int {
    stdio_uart_init_full(PICO_UART, PICO_UART_BAUD_RATE, PICO_UART_TX_PIN,
                         PICO_UART_RX_PIN);

    vLaunch();

    return 0;
}
