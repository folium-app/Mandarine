#pragma once

#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/device/spu/spu.h"

#else

#include "device/spu/spu.h"

#endif // TARGET_OS_IPHONE

namespace spu {
    int16_t interpolate(Voice &v, int pos, int i);
}  // namespace spu
