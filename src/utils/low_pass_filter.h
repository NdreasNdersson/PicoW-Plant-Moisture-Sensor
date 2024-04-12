#ifndef __UTILS__LOW_PASS_FILTER__
#define __UTILS__LOW_PASS_FILTER__

#include <cmath>
#define M_PI 3.14159265358979323846

template <typename T>
class LowPassFilter {
   public:
    LowPassFilter(float cut_off_frequency, float delta_time)
        : value_{}, e_pow_{0.0F}, first_value_{true} {
        e_pow_ = 1 - std::exp(-delta_time * 2 * M_PI * cut_off_frequency);
    }
    T update(T input) {
        if (first_value_) {
            first_value_ = false;
            value_ = input;
        }
        return value_ += (input - value_) * e_pow_;
    }

   private:
    T value_;
    float e_pow_;
    bool first_value_;
};

#endif
