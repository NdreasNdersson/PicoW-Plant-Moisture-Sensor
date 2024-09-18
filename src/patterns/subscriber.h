#ifndef PICO_REST_SENSOR_PATTERNS_SUBSCRIBER_H_
#define PICO_REST_SENSOR_PATTERNS_SUBSCRIBER_H_

template <typename Payload>
class Subscriber {
   public:
    Subscriber() = default;
    virtual ~Subscriber() = default;
    Subscriber(const Subscriber&) = delete;
    Subscriber(Subscriber&&) noexcept = delete;
    Subscriber& operator=(const Subscriber&) = delete;
    Subscriber& operator=(Subscriber&&) noexcept = delete;

    virtual void update(const Payload& payload) = 0;
};

#endif  // PICO_REST_SENSOR_PATTERNS_SUBSCRIBER_H_
