#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/device/dma/dma0_channel.h"
#include "avocado/device/mdec/mdec.h"

#else

#include "device/dma/dma0_channel.h"
#include "device/mdec/mdec.h"

#endif // TARGET_OS_IPHONE

namespace device::dma {
DMA0Channel::DMA0Channel(Channel channel, System* sys, mdec::MDEC* mdec) : DMAChannel(channel, sys), mdec(mdec) {}

void DMA0Channel::writeDevice(uint32_t data) { mdec->write(0, data); }

bool DMA0Channel::dataRequest() { return mdec->dataInRequest(); }
}  // namespace device::dma
