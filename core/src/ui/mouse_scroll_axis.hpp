#ifndef WINDOWER_UI_MOUSE_SCROLL_AXIS_HPP
#define WINDOWER_UI_MOUSE_SCROLL_AXIS_HPP

#include <cstdint>

namespace windower::ui
{

enum class mouse_scroll_axis : std::uint8_t
{
    vertical = 0,
    horizontal = 1,
};

}

#endif