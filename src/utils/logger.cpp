#include "logger.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "queue.h"

static QueueHandle_t g_queue = NULL;

void logger(const char *format, ...) {
    buffer_t buffer;
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    if (g_queue != NULL) {
        if (xQueueSend(g_queue, buffer, 0U) == errQUEUE_FULL) {
            printf("LOGGING QUEUE IS FULL!");
        }
    }
}

void init_queue() { g_queue = xQueueCreate(50, sizeof(buffer_t)); }

void print_task(void *params) {
    buffer_t buffer;
    while (true) {
        if (g_queue != NULL &&
            xQueueReceive(g_queue, buffer, portMAX_DELAY) == pdTRUE) {
            printf(buffer);
        }
    }
}
