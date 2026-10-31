#pragma once

#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/system.h"

#else

#include "system.h"

#endif // TARGET_OS_IPHONE

enum class PsfType { Main, MainLib, SecondaryLib };

bool loadPsf(System* sys, const std::string& file, PsfType type = PsfType::Main);
