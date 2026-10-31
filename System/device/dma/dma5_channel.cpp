#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/device/dma/dma5_channel.h"

#else

#include "device/dma/dma5_channel.h"

#endif // TARGET_OS_IPHONE

namespace device::dma {
DMA5Channel::DMA5Channel(Channel channel, System* sys) : DMAChannel(channel, sys) {}
}  // namespace device::dma
