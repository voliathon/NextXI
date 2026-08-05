#ifndef WINDOWER_UI_MOUSE_BUTTON_HPP
#define WINDOWER_UI_MOUSE_BUTTON_HPP

#include <cstdint>

namespace windower::ui
{

enum class mouse_button : std::uint8_t
{
    left = 0,
    right = 1,
    middle = 2,
    x1 = 3,
    x2 = 4,
};

}

#endif