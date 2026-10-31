#pragma once

#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/disc/position.h"
#include "avocado/disc/subchannel_q.h"

#else

#include "disc/position.h"
#include "disc/subchannel_q.h"

#endif // TARGET_OS_IPHONE

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace disc {
enum class TrackType { DATA, AUDIO, INVALID };
typedef std::vector<uint8_t> Data;
typedef std::vector<uint8_t> Subcode;
typedef std::pair<Data, TrackType> Sector;

struct Disc {
    virtual ~Disc() = default;
    virtual Sector read(Position pos) = 0;

    virtual std::string getFile() const = 0;
    virtual size_t getTrackCount() const = 0;
    virtual int getTrackByPosition(Position pos) const = 0;
    virtual Position getTrackBegin(int track) const = 0;  // MMSSFF of track first sector
    virtual Position getTrackStart(int track) const = 0;  // MMSSFF of track index1
    virtual Position getTrackLength(int track) const = 0;
    virtual Position getDiskSize() const = 0;

    SubchannelQ getSubQ(Position pos);
    bool loadSubchannel(const std::string& path);

   private:
    std::unordered_map<Position, SubchannelQ> modifiedQ;
    bool loadLsd(const std::vector<uint8_t>& lsd);
    bool loadSbi(const std::vector<uint8_t>& sbi);
};
}  // namespace disc
