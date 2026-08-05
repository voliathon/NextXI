#ifndef WINDOWER_ENUMS_HPP
#define WINDOWER_ENUMS_HPP

#include <cstdint>

namespace windower
{
    enum class window_type : std::int32_t
    {
        borderless = 0,
        window = 1,
        full_screen = 2,
    };

    enum class texture_compression : std::int32_t
    {
        high = 0,
        low = 1,
        uncompressed = 2,
    };

    enum class environment_animation : std::int32_t
    {
        off = 0,
        normal = 1,
        smooth = 2,
    };

    enum class font_type : std::int32_t
    {
        compressed = 0,
        uncompressed = 1,
        high_quality = 2,
    };

    enum class mouse_button
    {
        left,
        right,
        middle,
    };
}

#endif