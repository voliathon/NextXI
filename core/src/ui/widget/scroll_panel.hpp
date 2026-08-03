#ifndef WINDOWER_UI_WIDGET_SCROLL_PANEL_HPP
#define WINDOWER_UI_WIDGET_SCROLL_PANEL_HPP

#include "ui/context.hpp"
#include "ui/dimension.hpp"
#include "ui/id.hpp"
#include "ui/vector.hpp"

#include <cstdint>

namespace windower::ui::widget
{

enum class scroll_bar_visibility : std::uint8_t
{
    hidden    = 0,
    visible   = 1,
    automatic = 2,
};

class scroll_panel_state
{
public:
    dimension canvas_size;
    scroll_bar_visibility visibility_horizontal =
        scroll_bar_visibility::automatic;
    scroll_bar_visibility visibility_vertical =
        scroll_bar_visibility::automatic;
    float line_height = 16;
    vector offset;
};

void begin_scroll_panel(
    context& ctx, id id, scroll_panel_state& state) noexcept;
void end_scroll_panel(context& ctx) noexcept;

}

#endif