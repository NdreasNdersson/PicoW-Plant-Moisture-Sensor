#include <gmock/gmock.h>

#include "hal/freertos_interface.h"
#include "semphr.h"

class MockFreertosInterface : public FreertosInterface {
   public:
    MOCK_METHOD(SemaphoreHandle_t, semaphore_create_mutex, (), (override));
    MOCK_METHOD(BaseType_t, semaphore_take,
                (SemaphoreHandle_t semaphore, TickType_t ticks_to_wait),
                (override));
    MOCK_METHOD(void, semaphore_give, (SemaphoreHandle_t semaphore),
                (override));
};
