#ifndef __UTILS__BUTTON__BUTTON__
#define __UTILS__BUTTON__BUTTON__

#include <list>

#include "patterns/publisher.h"
#include "patterns/subscriber.h"
#include "pico/types.h"

using pin_t = uint;

class Button : public Publisher {
   public:
    Button();
    Button(pin_t pin);
    void attach(Subscriber *subscriber) override;
    void detach(Subscriber *subscriber) override;
    void notify() override;

    pin_t get_pin();

   private:
    pin_t pin_;
    std::list<Subscriber *> list_subscribers_;
};

#endif
