#ifndef WINDOWER_UI_WIDGET_SLIDER_HPP
#define WINDOWER_UI_WIDGET_SLIDER_HPP

#include "ui/context.hpp"
#include "ui/direction.hpp"
#include "ui/id.hpp"

namespace windower::ui::widget
{

float slider(
    context& ctx, id id, float value, float min, float max,
    direction direction = direction::left_to_right) noexcept;

float slider(
    context& ctx, id id, float value, float min, float max, color fill_color,
    direction direction = direction::left_to_right) noexcept;

}

#endif