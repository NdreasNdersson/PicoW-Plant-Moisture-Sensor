#include "hal/freertos_interface_impl.h"

#include "FreeRTOS.h"
#include "semphr.h"

auto FreertosInterfaceImpl::semaphore_create_mutex() -> SemaphoreHandle_t {
    return xSemaphoreCreateMutex();
}

auto FreertosInterfaceImpl::semaphore_take(SemaphoreHandle_t semaphore,
                                           TickType_t ticks_to_wait)
    -> BaseType_t {
    return xSemaphoreTake(semaphore, ticks_to_wait);
}

void FreertosInterfaceImpl::semaphore_give(SemaphoreHandle_t semaphore) {
    xSemaphoreGive(semaphore);
}
