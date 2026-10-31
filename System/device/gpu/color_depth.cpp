#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/device/gpu/color_depth.h"

#else

#include "device/gpu/color_depth.h"

#endif // TARGET_OS_IPHONE

ColorDepth bitsToDepth(int bits) {
    switch (bits) {
        case 4: return ColorDepth::BIT_4;
        case 8: return ColorDepth::BIT_8;
        case 16: return ColorDepth::BIT_16;
        default: return ColorDepth::NONE;
    }
}
