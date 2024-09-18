#ifndef PICO_REST_SENSOR_PATTERNS_PUBLISHER_H_
#define PICO_REST_SENSOR_PATTERNS_PUBLISHER_H_

#include "subscriber.h"

template <typename Payload>
class Publisher {
   public:
    Publisher() = default;
    virtual ~Publisher() = default;
    Publisher(const Publisher&) = delete;
    Publisher(Publisher&&) noexcept = delete;
    Publisher& operator=(const Publisher&) = delete;
    Publisher& operator=(Publisher&&) noexcept = delete;

    virtual void attach(Subscriber<Payload>* subscriber) = 0;
    virtual void detach(Subscriber<Payload>* subscriber) = 0;
    virtual void notify(const Payload& payload) = 0;
};
#endif  // PICO_REST_SENSOR_PATTERNS_PUBLISHER_H_
