#pragma once
#include <tuple>
#include "device/spu/spu.h"

namespace spu {
std::tuple<int16_t, int16_t> doReverb(SPU* spu, std::tuple<int16_t, int16_t> input);
}  // namespace spu
