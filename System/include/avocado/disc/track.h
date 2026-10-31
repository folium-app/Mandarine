#pragma once

#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/disc/disc.h"
#include "avocado/disc/position.h"

#else

#include "disc/disc.h"
#include "disc/position.h"

#endif // TARGET_OS_IPHONE

#include <optional>

namespace disc {
struct Track {
    inline static const int SECTOR_SIZE = 2352;

    std::string filename;
    int number = 0;
    disc::TrackType type;

    Position pregap;                 // (Size) silence not included in image
    std::optional<Position> index0;  // (Position) Audio "before" start, same as pause
    Position index1;                 // (Position) Start of data/audio
    Position postgap;                // (Size) "silence" included in image

    size_t offset;  // aka offset in file in sectors (frames)
    size_t frames;  // aka size in sectors

    Track() {
        pregap = {0, 0, 0};
        index1 = {0, 0, 0};
        postgap = {0, 0, 0};

        frames = 0;
    }

    Position pause() const {
        if (index0) {
            return (index1 - *index0);
        }
        return Position(0, 0, 0);
    }

    Position start() const { return pregap + pause(); }
};
}  // namespace disc
