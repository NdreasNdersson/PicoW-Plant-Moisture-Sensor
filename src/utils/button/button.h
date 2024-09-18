#ifndef PICO_REST_SENSOR_UTILS_BUTTON_BUTTON_H_
#define PICO_REST_SENSOR_UTILS_BUTTON_BUTTON_H_

#include <list>

#include "patterns/publisher.h"
#include "patterns/subscriber.h"
#include "pico/types.h"

using pin_t = uint;

class Button : public Publisher<int> {
   public:
    Button() = default;
    explicit Button(pin_t pin);
    void attach(Subscriber<int> *subscriber) override;
    void detach(Subscriber<int> *subscriber) override;
    void notify(const int &) override;

    auto get_pin();

   private:
    pin_t pin_;
    std::list<Subscriber<int> *> list_subscribers_;
};

#endif  // PICO_REST_SENSOR_UTILS_BUTTON_BUTTON_H_
