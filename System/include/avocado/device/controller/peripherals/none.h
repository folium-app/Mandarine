#pragma once

#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/device/device.h"
#include "avocado/device/controller/peripherals/abstract_device.h"

#else

#include "device/device.h"
#include "device/controller/peripherals/abstract_device.h"

#endif // TARGET_OS_IPHONE

#include <string>

namespace peripherals {
struct None : public AbstractDevice {
    None(int port);
    uint8_t handle(uint8_t byte) override;
};
};  // namespace peripherals
