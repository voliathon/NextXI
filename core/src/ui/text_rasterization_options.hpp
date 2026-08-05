#ifndef WINDOWER_UI_TEXT_RASTERIZATION_OPTIONS_HPP
#define WINDOWER_UI_TEXT_RASTERIZATION_OPTIONS_HPP

#include "ui/color.hpp"

#include <array>

namespace windower::ui
{

enum class text_rasterization_flags : std::int32_t
{
    none           = 0,
    clip_to_bounds = 1,
    show_cursor    = 2,
};

class text_rasterization_options
{
public:
    color fill_color               = colors::white;
    color stroke_color             = colors::transparent;
    float stroke_width             = 0.f;
    text_rasterization_flags flags = text_rasterization_flags::none;
};

}

#endif