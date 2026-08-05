#ifndef WINDOWER_UI_DIRECTION_HPP
#define WINDOWER_UI_DIRECTION_HPP

#include <cstdint>

namespace windower::ui
{

enum class direction : std::uint8_t
{
    left_to_right = 0,
    right_to_left = 1,
    bottom_to_top = 2,
    top_to_bottom = 3,
};

}

#endif