#pragma once

#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/device/dma/dma_channel.h"

#else

#include "device/dma/dma_channel.h"

#endif // TARGET_OS_IPHONE

namespace spu {
struct SPU;
}

namespace device::dma {
class DMA4Channel : public DMAChannel {
    spu::SPU *spu = nullptr;

    uint32_t readDevice() override;
    void writeDevice(uint32_t data) override;

   public:
    DMA4Channel(Channel channel, System *sys, spu::SPU *spu);
};
}  // namespace device::dma
