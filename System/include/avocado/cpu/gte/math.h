#pragma once

#include <array>
#include <cstdint>

namespace gte {

    template <typename R, typename G = R, typename B = G, typename C = B>
    struct Color {
        R r;
        G g;
        B b;
        C c;

        template <class Archive>
        void serialize(Archive& archive) {
            archive(r, g, b, c);
        }
    };

    using Matrix = std::array<std::array<int16_t, 0x03>, 0x03>;

    template <typename X, typename Y = X, typename Z = Y>
    struct Vector {
        X x, r;
        Y y, g;
        Z z, b;

        template <class Archive>
        void serialize(Archive& archive) {
            archive(x, y, z);
            archive(r, g, b);
        }
    };

    Vector<int16_t> to_vector(std::array<int16_t, 0x04>& /* values */);

}; // namespace gte