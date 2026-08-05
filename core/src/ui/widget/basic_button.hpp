#ifndef WINDOWER_UI_WIDGET_BASIC_BUTTON_HPP
#define WINDOWER_UI_WIDGET_BASIC_BUTTON_HPP

#include "ui/context.hpp"
#include "ui/id.hpp"
#include "ui/mouse_button.hpp"
#include "ui/vector.hpp"

#include <cstdint>

namespace windower::ui::widget
{

class button_state
{
public:
    bool hot     = false;
    bool active  = false;
    bool pressed = false;
    bool clicked = false;
    vector drag_offset;
    vector drag_position;
    std::uint64_t repeat_count = 0;
    mouse_button button        = mouse_button::left;

    constexpr operator bool() const noexcept
    {
        return clicked && button == mouse_button::left;
    }
};

button_state basic_button(context& ctx, id id) noexcept;

}

#endif