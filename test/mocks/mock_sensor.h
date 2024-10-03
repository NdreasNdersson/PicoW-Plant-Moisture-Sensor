#include <gmock/gmock.h>

#include "patterns/subscriber.h"
#include "sensors/sensor.h"
#include "sensors/sensor_config.h"

class MockSensor : public Sensor {
   public:
    MOCK_METHOD(SensorReadStatus, read, (sensor_config_t & config), (override));
    MOCK_METHOD(void, attach, (Subscriber<Measurement_t> * subscriber),
                (override));
    MOCK_METHOD(void, detach, (Subscriber<Measurement_t> * subscriber),
                (override));
    MOCK_METHOD(void, notify, (const Measurement_t&), (override));
};
