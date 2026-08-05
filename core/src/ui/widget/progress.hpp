#ifndef WINDOWER_UI_WIDGET_PROGRESS_HPP
#define WINDOWER_UI_WIDGET_PROGRESS_HPP

#include "ui/color.hpp"
#include "ui/context.hpp"
#include "ui/direction.hpp"

#include <span>

namespace windower::ui::widget
{

class progress_entry
{
public:
    float value;
    float max;
    color color;
};

void progress(
    context& ctx, float value, float max = 1.f,
    direction direction = direction::left_to_right) noexcept;

void progress(
    context& ctx, float value, float max, color color,
    direction direction = direction::left_to_right) noexcept;

void progress(
    context& ctx, std::span<progress_entry const> entries,
    direction direction = direction::left_to_right) noexcept;

}

#endif