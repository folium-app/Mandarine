#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/system.h"
#include "avocado/device/cache_control.h"

#else

#include "system.h"
#include "device/cache_control.h"

#endif // TARGET_OS_IPHONE

CacheControl::CacheControl(System* sys) : sys(sys) { reset(); }

void CacheControl::reset() { cache._reg = 0; }

uint32_t CacheControl::read(uint32_t address) { return cache._reg; }

void CacheControl::write(uint32_t address, uint32_t data) {
    cache._reg = data;
    sys->cpu->icacheEnabled = cache.icacheEnable;
}
