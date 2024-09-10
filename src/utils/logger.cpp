#include "logger.h"

#include <cstdio>

#include "FreeRTOS.h"
#include "queue.h"

static QueueHandle_t queue = nullptr;

void logger(const char *format, ...) {
    buffer_t buffer;
    va_list args;
    va_start(args, format);
    auto result{vsprintf(buffer, format, args)};
    va_end(args);

    if ((result >= 0) && (queue != nullptr)) {
        if (xQueueSend(queue, buffer, 0U) == errQUEUE_FULL) {
            printf("LOGGING QUEUE IS FULL!");
        }
    }
}

void init_queue() { queue = xQueueCreate(64, sizeof(buffer_t)); }

void print_task(void * /*params*/) {
    buffer_t buffer;
    while (true) {
        if (queue != nullptr &&
            xQueueReceive(queue, buffer, portMAX_DELAY) == pdTRUE) {
            printf("%s\n", buffer);
        }
    }
}
