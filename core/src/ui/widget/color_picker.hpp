#ifndef WINDOWER_UI_WIDGET_COLOR_PICKER_HPP
#define WINDOWER_UI_WIDGET_COLOR_PICKER_HPP

#include "ui/color.hpp"
#include "ui/context.hpp"
#include "ui/id.hpp"

namespace windower::ui::widget
{

color color_picker(
    context& ctx, id id, color value, bool alpha = true) noexcept;

}

#endif