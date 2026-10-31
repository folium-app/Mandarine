//
//  tools.h
//  Mandarine
//
//  Created by Jarrod Norwell on 17/10/2026.
//

#pragma once

#include <memory>
#include <string>

struct System;
namespace tools {
    void bootstrap(std::unique_ptr<System>& /* system */);
    std::unique_ptr<System> reset();

    void load(std::unique_ptr<System>& /* system */, const std::string& /* path */);

    template <bool /* write */>
    bool handle_memory_card(std::unique_ptr<System>& /* system */, const int /* slot */, const bool /* force */ = false);
} // namespace tools