#ifndef __PATTERNS__PUBLISHER__
#define __PATTERNS__PUBLISHER__

#include "subscriber.h"

class Publisher {
   public:
    Publisher() = default;
    virtual ~Publisher() = default;
    Publisher(const Publisher&) = default;
    Publisher(Publisher&&) = default;
    Publisher& operator=(const Publisher&) = default;
    Publisher& operator=(Publisher&&) = default;

    virtual void attach(Subscriber* subscriber) = 0;
    virtual void detach(Subscriber* subscriber) = 0;
    virtual void notify() = 0;
};
#endif
