#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/device/controller/peripherals/abstract_device.h"

#else

#include "device/controller/peripherals/abstract_device.h"

#endif // TARGET_OS_IPHONE

namespace peripherals {
AbstractDevice::AbstractDevice(Type type, int port) : type(type), port(port) {}

AbstractDevice::~AbstractDevice() {}

bool AbstractDevice::getAck() { return state != 0; }

void AbstractDevice::resetState() { state = 0; }
void AbstractDevice::update() {}
};  // namespace peripherals
