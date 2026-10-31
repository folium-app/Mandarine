#pragma once

#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/device/dma/dma_channel.h"

#else

#include "device/dma/dma_channel.h"

#endif // TARGET_OS_IPHONE

namespace device::cdrom {
class CDROM;
}

namespace device::dma {
class DMA3Channel : public DMAChannel {
    device::cdrom::CDROM* cdrom;

    uint32_t readDevice() override;

   public:
    DMA3Channel(Channel channel, System* sys, device::cdrom::CDROM* cdrom);
};
}  // namespace device::dma
