#pragma once

#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/disc/disc.h"
#include "avocado/disc/format/ecm.h"

#else

#include "disc/disc.h"
#include "disc/format/ecm.h"

#endif // TARGET_OS_IPHONE

#include <cstdio>
#include <memory>
#include <vector>

namespace disc::format {
class EcmParser {
   private:
    unique_ptr_file f;
    std::vector<uint8_t> data;
    uint8_t frame[2352];

    uint32_t calculateEDC(size_t addr, size_t size) const;
    void adjustEDC(size_t addr, size_t size);
    void adjustSync();
    void copySubheader();
    void calculateAndAdjustECC();
    void computeECCblock(uint8_t* src, uint32_t majorCount, uint32_t minorCount, uint32_t majorMult, uint32_t minorInc, uint8_t* dst);
    void handleMode1();
    void handleMode2Form1();
    void handleMode2Form2();

   public:
    std::unique_ptr<Ecm> parse(const char* file);
};
}  // namespace disc::format
