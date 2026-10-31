#pragma once

#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/device/spu/spu.h"

#else

#include "device/spu/spu.h"

#endif // TARGET_OS_IPHONE

#include <tuple>

namespace spu {
std::tuple<int16_t, int16_t> doReverb(SPU* spu, std::tuple<int16_t, int16_t> input);
}  // namespace spu
