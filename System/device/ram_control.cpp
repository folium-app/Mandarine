#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/config.h"
#include "avocado/device/ram_control.h"

#else

#include "config.h"
#include "device/ram_control.h"

#endif // TARGET_OS_IPHONE

#include <fmt/core.h>

RamControl::RamControl() {
    verbose = config.debug.log.memoryControl > 0;
    reset();
}

void RamControl::reset() {
    ramSize._reg = 0x00000b88;  // TODO: Check reset value
}

uint8_t RamControl::read(uint32_t address) { return ramSize.read(address); }

void RamControl::write(uint32_t address, uint8_t data) {
    ramSize.write(address, data);
    if (verbose && address == 0x3) {
        fmt::print("[RAMCTRL] RAM_SIZE: 0x{:08x}\n", ramSize._reg);
    }
}
