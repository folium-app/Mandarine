#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/device/dma/dma1_channel.h"
#include "avocado/device/mdec/mdec.h"

#else

#include "device/dma/dma1_channel.h"
#include "device/mdec/mdec.h"

#endif // TARGET_OS_IPHONE

namespace device::dma {
DMA1Channel::DMA1Channel(Channel channel, System* sys, mdec::MDEC* mdec) : DMAChannel(channel, sys), mdec(mdec) {}

uint32_t DMA1Channel::readDevice() { return mdec->read(0); }

bool DMA1Channel::dataRequest() { return mdec->dataOutRequest(); }

bool DMA1Channel::hack_supportChoppedTransfer() const { return true; }
}  // namespace device::dma
