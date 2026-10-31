#pragma once

#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/device/dma/dma_channel.h"

#else

#include "device/dma/dma_channel.h"

#endif // TARGET_OS_IPHONE

namespace mdec {
class MDEC;
};

namespace device::dma {
class DMA0Channel : public DMAChannel {
    mdec::MDEC* mdec;

    void writeDevice(uint32_t data) override;
    bool dataRequest() override;

   public:
    DMA0Channel(Channel channel, System* sys, mdec::MDEC* mdec);
};
}  // namespace device::dma
