#ifndef PICO_REST_SENSOR_PATTERNS_SUBSCRIBER_H_
#define PICO_REST_SENSOR_PATTERNS_SUBSCRIBER_H_

class Subscriber {
   public:
    Subscriber() = default;
    virtual ~Subscriber() = default;
    Subscriber(const Subscriber&) = default;
    Subscriber(Subscriber&&) = default;
    Subscriber& operator=(const Subscriber&) = default;
    Subscriber& operator=(Subscriber&&) = default;

    virtual void update() = 0;
};

#endif  // PICO_REST_SENSOR_PATTERNS_SUBSCRIBER_H_
