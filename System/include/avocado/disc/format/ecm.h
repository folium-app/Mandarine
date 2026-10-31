#pragma once

#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/disc/disc.h"
#include "avocado/disc/position.h"
#include "avocado/disc/track.h"
#include "avocado/utils/file.h"

#else

#include "disc/disc.h"
#include "disc/position.h"
#include "disc/track.h"
#include "utils/file.h"

#endif // TARGET_OS_IPHONE

#include <cstdio>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace disc::format {
struct Ecm : public Disc {
   private:
    std::string file;
    std::vector<uint8_t> data;

   public:
    Ecm(std::string file, std::vector<uint8_t> data);

    std::string getFile() const override;
    Position getDiskSize() const override;
    size_t getTrackCount() const override;
    Position getTrackBegin(int track) const override;
    Position getTrackStart(int track) const override;
    Position getTrackLength(int track) const override;
    int getTrackByPosition(Position pos) const override;

    disc::Sector read(Position pos) override;
};
}  // namespace disc::format
