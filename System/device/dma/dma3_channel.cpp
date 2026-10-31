#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/device/cdrom/cdrom.h"
#include "avocado/device/dma/dma3_channel.h"
#include "avocado/utils/file.h"

#else

#include "device/cdrom/cdrom.h"
#include "device/dma/dma3_channel.h"
#include "utils/file.h"

#endif // TARGET_OS_IPHONE

namespace device::dma {
uint32_t DMA3Channel::readDevice() {
    uint32_t data = 0;
    data |= cdrom->readByte() << 0;
    data |= cdrom->readByte() << 8;
    data |= cdrom->readByte() << 16;
    data |= cdrom->readByte() << 24;
    return data;
}

DMA3Channel::DMA3Channel(Channel channel, System* sys, device::cdrom::CDROM* cdrom) : DMAChannel(channel, sys), cdrom(cdrom) {}

}  // namespace device::dma
