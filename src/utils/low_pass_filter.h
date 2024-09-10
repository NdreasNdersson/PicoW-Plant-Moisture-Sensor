#ifndef PICO_REST_SENSOR_UTILS_LOW_PASS_FILTER_H_
#define PICO_REST_SENSOR_UTILS_LOW_PASS_FILTER_H_

#include <cmath>

constexpr float PI{3.14159274101257324219};

template <typename T>
class LowPassFilter {
   public:
    LowPassFilter(float cut_off_frequency, float delta_time)
        : value_{}, e_pow_{0.0F}, first_value_{true} {
        e_pow_ = 1 - expf(-delta_time * 2.0f * PI * cut_off_frequency);
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

#endif  // PICO_REST_SENSOR_UTILS_LOW_PASS_FILTER_H_
