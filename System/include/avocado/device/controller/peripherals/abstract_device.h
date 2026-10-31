#pragma once

#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/device/device.h"

#else

#include "device/device.h"

#endif // TARGET_OS_IPHONE

namespace peripherals {
enum class Type { None, Digital, Analog, Mouse, MemoryCard };
struct AbstractDevice {
    Type type;
    int port;  // Physical port number (numbered from 1..n)
    int state = 0;

    AbstractDevice(Type type, int port);
    bool getAck();
    void resetState();

    // Handle serial communication with console
    virtual uint8_t handle(uint8_t byte) = 0;

    // Update key state
    virtual void update();

    virtual ~AbstractDevice();
};
};  // namespace peripherals
