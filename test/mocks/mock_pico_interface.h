#include <gmock/gmock.h>

#include <cstdint>

#include "hal/pico_interface.h"

class MockPicoInterface : public PicoInterface {
   public:
    MOCK_METHOD(bool, store_to_flash,
                (uint32_t flash_offs, const uint8_t *data, size_t count),
                (override));
    MOCK_METHOD(bool, erase_flash, (uint32_t flash_offs, size_t count),
                (override));
};
