#pragma once

#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/device/dma/dma_channel.h"

#else

#include "device/dma/dma_channel.h"

#endif // TARGET_OS_IPHONE

namespace gpu {
class GPU;
}

namespace device::dma {
class DMA2Channel : public DMAChannel {
    gpu::GPU *gpu;

    uint32_t readDevice() override;
    void writeDevice(uint32_t data) override;

   public:
    DMA2Channel(Channel channel, System *sys, gpu::GPU *gpu);
};
}  // namespace device::dma
