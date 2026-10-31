#pragma once

#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/system.h"

#else

#include "system.h"

#endif // TARGET_OS_IPHONE

#include <string>

namespace GpuDrawList {
extern int framesToCapture;
extern int currentFrame;

bool load(System *sys, const std::string &path);
bool save(System *sys, const std::string &path);
void replayCommands(gpu::GPU *gpu, int to = -1);
void dumpInitialState(gpu::GPU *gpu);
}  // namespace GpuDrawList
