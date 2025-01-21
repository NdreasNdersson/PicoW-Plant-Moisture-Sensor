#ifndef PICO_REST_SENSOR_FREERTOS_INTERFACE_IMPL_H_
#define PICO_REST_SENSOR_FREERTOS_INTERFACE_IMPL_H_

#include "hal/freertos_interface.h"
#include "semphr.h"

class FreertosInterfaceImpl : public FreertosInterface {
   public:
    auto semaphore_create_mutex() -> SemaphoreHandle_t override;
    auto semaphore_take(SemaphoreHandle_t semaphore, TickType_t ticks_to_wait)
        -> BaseType_t;
    void semaphore_give(SemaphoreHandle_t semaphore);
};

#endif  // PICO_REST_SENSOR_FREERTOS_INTERFACE_IMPL_H_
