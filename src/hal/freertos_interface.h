#ifndef PICO_REST_SENSOR_FREERTOS_INTERFACE_H_
#define PICO_REST_SENSOR_FREERTOS_INTERFACE_H_

#include "FreeRTOS.h"
#include "semphr.h"

class FreertosInterface {
   public:
    virtual auto semaphore_create_mutex() -> SemaphoreHandle_t = 0;
    virtual auto semaphore_take(SemaphoreHandle_t semaphore,
                                TickType_t ticks_to_wait) -> BaseType_t = 0;
    virtual void semaphore_give(SemaphoreHandle_t semaphore) = 0;
};

#endif  // PICO_REST_SENSOR_FREERTOS_INTERFACE_H_
