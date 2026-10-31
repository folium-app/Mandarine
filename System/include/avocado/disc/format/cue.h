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

namespace disc {
namespace format {
struct Cue : public Disc {
    std::string file;
    std::vector<Track> tracks;

    Cue() = default;
    Cue(Cue& cue) : file(cue.file), tracks(cue.tracks) {}
    static std::unique_ptr<Cue> fromBin(const char* file);

    std::string getFile() const override;
    Position getDiskSize() const override;
    size_t getTrackCount() const override;
    Position getTrackBegin(int track) const override;
    Position getTrackStart(int track) const override;
    Position getTrackLength(int track) const override;
    int getTrackByPosition(Position pos) const override;

    disc::Sector read(Position pos) override;

   private:
    std::unordered_map<std::string, unique_ptr_file> files;
};
}  // namespace format
}  // namespace disc
