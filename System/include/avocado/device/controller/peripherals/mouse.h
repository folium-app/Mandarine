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
class Mouse : public AbstractDevice {
    std::string path;
    bool left = false, right = false;
    int8_t x = 0, y = 0;

   public:
    Mouse(int port);
    uint8_t handle(uint8_t byte) override;
    void update() override;
};
};  // namespace peripherals
