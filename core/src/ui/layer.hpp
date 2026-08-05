#ifndef WINDOWER_UI_LAYER_HPP
#define WINDOWER_UI_LAYER_HPP

#include <cstdint>

namespace windower::ui
{

enum class layer : std::uint8_t
{
    layout = 0,
    screen = 1,
    world = 2,
};

}

#endif