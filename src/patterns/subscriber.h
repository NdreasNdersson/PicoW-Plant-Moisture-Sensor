#ifndef __PATTERNS__SUBSCRIBER__
#define __PATTERNS__SUBSCRIBER__

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
#endif
