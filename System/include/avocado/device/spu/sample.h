#pragma once

#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/utils/math.h"

#else

#include "utils/math.h"

#endif // TARGET_OS_IPHONE

#include <cstdint>

// Basically an overflow safe int16_t
struct Sample {
    int16_t value;

    Sample operator*(const int16_t& b) const {  //
        return (value * b) >> 15;
    }
    Sample& operator*=(const int16_t& b) {
        value = (value * b) >> 15;
        return *this;
    }
    Sample operator+(const int16_t& b) const {  //
        return clamp<int32_t>(value + b, INT16_MIN, INT16_MAX);
    }
    Sample& operator+=(const int16_t& b) {
        value = clamp<int32_t>(value + b, INT16_MIN, INT16_MAX);
        return *this;
    }
    Sample operator-(const int16_t& b) const {  //
        return clamp<int32_t>(value - b, INT16_MIN, INT16_MAX);
    }
    Sample& operator-=(const int16_t& b) {
        value = clamp<int32_t>(value - b, INT16_MIN, INT16_MAX);
        return *this;
    }
    operator int16_t() const {  //
        return value;
    }
    Sample(int16_t v) : value(v) {}
    Sample() : value(0) {}
};
