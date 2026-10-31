#pragma once

#include <cstdint>

namespace gte {

    namespace mvmva {

        enum class TranslationVector : uint32_t {
            TR = 0,
            BK = 1,
            FC = 2,
            NONE = 3
        };

        enum class MultiplyVector : uint32_t {
            V0 = 0,
            V1 = 1,
            V2 = 2,
            IR = 3
        };

        enum class MultiplyMatrix : uint32_t {
            ROTATION = 0,
            LIGHT = 1,
            COLOR = 2,
            RESERVED = 3
        };

    }; // namespace mvmva

    enum class ShiftFraction : uint32_t {
        NO_FRACTION = 0,
        TWELVE_BIT_FRACTION = 1
    };

    struct Command {
        uint32_t cop2 : 7;
        uint32_t fake_gte_command_number : 5;
        gte::ShiftFraction sf : 1;
        gte::mvmva::MultiplyMatrix mvmva_multiply_matrix : 2;
        gte::mvmva::MultiplyVector mvmva_multiply_vector : 2;
        gte::mvmva::TranslationVector mvmva_translation_vector : 2;
        uint32_t zero1 : 2;
        uint32_t lm : 1;
        uint32_t zero0 : 4;
        uint32_t real_gte_command_number : 6;

        constexpr bool ShiftFraction() const {
            return sf == gte::ShiftFraction::TWELVE_BIT_FRACTION;
        };
        
    };

}; //namespace gte