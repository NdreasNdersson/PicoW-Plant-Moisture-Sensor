#ifndef __PATTERNS__PUBLISHER__
#define __PATTERNS__PUBLISHER__

#include "subscriber.h"

class Publisher {
   public:
    virtual ~Publisher(){};
    virtual void attach(Subscriber *subscriber) = 0;
    virtual void detach(Subscriber *subscriber) = 0;
    virtual void notify() = 0;
};
#endif
