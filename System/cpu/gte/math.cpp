#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/cpu/gte/math.h"

#else

#include "cpu/gte/math.h"

#endif // TARGET_OS_IPHONE

gte::Vector<int16_t> gte::to_vector(std::array<int16_t, 0x04>& values) {
    return gte::Vector{
        values.at(1),
        values.at(2),
        values.at(3)
    };
};