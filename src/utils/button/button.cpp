#include "button.h"

#include "patterns/subscriber.h"

Button::Button(pin_t pin) : pin_{pin}, list_subscribers_{} {}

void Button::attach(Subscriber<int> *subscriber) {
    list_subscribers_.push_back(subscriber);
}

void Button::detach(Subscriber<int> *subscriber) {
    list_subscribers_.remove(subscriber);
}

void Button::notify(const int &value) {
    for (auto subsriber : list_subscribers_) {
        subsriber->update(value);
    }
}

auto Button::get_pin() { return pin_; }
