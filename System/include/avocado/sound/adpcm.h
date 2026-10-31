#pragma once

#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/utils/cd.h"

#else

#include "utils/cd.h"

#endif // TARGET_OS_IPHONE

#include <array>
#include <vector>

#include <fmt/core.h>

namespace adpcm {

    namespace loop {

        enum class End : uint8_t {
            NO_CHANGE = 0,
            SET_ENDX_FLAG_AND_JUMP = 1
        };

        enum class Repeat : uint8_t {
            FORCE_RELEASE_AND_SET_ADSR_LEVEL_TO_ZERO = 0
        };

        enum class Start : uint8_t {
            NO_CHANGE = 0,
            COPY_CURRENT_ADDRESS = 1
        };

    }; // namespace loop

    struct flag {
        adpcm::loop::End loop_end : 1;
        adpcm::loop::Repeat loop_repeat : 1;
        adpcm::loop::Start loop_start : 1;
        uint8_t reserved : 5;

        constexpr bool LoopEnd() const {
            return loop_end == adpcm::loop::End::SET_ENDX_FLAG_AND_JUMP;
        };

        constexpr bool LoopRepeat() const {
            return loop_repeat not_eq adpcm::loop::Repeat::FORCE_RELEASE_AND_SET_ADSR_LEVEL_TO_ZERO && loop_end == adpcm::loop::End::SET_ENDX_FLAG_AND_JUMP;
        };

        constexpr bool LoopStart() const {
            return loop_start == adpcm::loop::Start::COPY_CURRENT_ADDRESS;
        };
    };

    std::vector<int16_t> decode(std::array<uint8_t, 16> /* buffer */, std::array<int32_t, 2>& /* previous_sample */);


    std::vector<std::pair<int16_t, int16_t>> decodeXA(uint8_t buffer[128 * 18], cd::Codinginfo codinginfo);
}; // namespace adpcm