#ifndef PICO_REST_SENSOR_UTILS_LOGGER_H_
#define PICO_REST_SENSOR_UTILS_LOGGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

typedef char buffer_t[512];

void logger(const char *format, ...);
void init_queue();
void print_task(void *params);

#ifdef __cplusplus
}
#endif

#endif  // PICO_REST_SENSOR_UTILS_LOGGER_H_
