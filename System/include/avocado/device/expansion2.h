#pragma once

#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/device/device.h"

#else

#include "device/device.h"

#endif // TARGET_OS_IPHONE

class Expansion2 {
    static const uint32_t BASE_ADDRESS = 0x1F802000;
    uint8_t post;

   public:
    Expansion2();
    void reset();
    uint8_t read(uint32_t address);
    void write(uint32_t address, uint8_t data);
};
