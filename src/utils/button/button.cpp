#include "button.h"

#include "patterns/subscriber.h"

Button::Button() : pin_{} {}
Button::Button(pin_t pin) : pin_{pin} {}

void Button::attach(Subscriber *subscriber) {
    list_subscribers_.push_back(subscriber);
}

void Button::detach(Subscriber *subscriber) {
    list_subscribers_.remove(subscriber);
}

void Button::notify() {
    for (auto subsriber : list_subscribers_) {
        subsriber->update();
    }
}

pin_t Button::get_pin() { return pin_; }
