#include "ui/widget/basic_button.hpp"

#include "ui/context.hpp"
#include "ui/mouse.hpp"
#include "ui/rectangle.hpp"
#include "ui/vector.hpp"

namespace windower::ui::widget
{
namespace
{

class internal_button_state
{
public:
    vector drag_start;
    vector drag_position;
    std::uint64_t last_hold_count;
    std::uint64_t repeat_count;
    mouse_button button;
};

}

button_state basic_button(context& ctx, id id) noexcept
{
    ctx.use_id(id);

    auto const position = ctx.bounds().position();

    button_state state{};
    state.drag_position = position;

    if (ctx.enabled())
    {
        state.hot = ctx.hit_test(id);

        auto const& mouse = ctx.mouse();
        auto const mouse_position =
            ctx.to_widget(mouse.position()).value_or(vector{});

        if (ctx.is_inactive() && state.hot)
        {
            if (mouse.was_pressed(mouse_button::left))
            {
                ctx.activate(id);
                ctx.emplace_active_state<internal_button_state>(
                    id, mouse_position, position, -1, 0, mouse_button::left);
            }
            else if (mouse.was_pressed(mouse_button::right))
            {
                ctx.activate(id);
                ctx.emplace_active_state<internal_button_state>(
                    id, mouse_position, position, -1, 0, mouse_button::right);
            }
            else if (mouse.was_pressed(mouse_button::middle))
            {
                ctx.activate(id);
                ctx.emplace_active_state<internal_button_state>(
                    id, mouse_position, position, -1, 0, mouse_button::middle);
            }
        }

        if (ctx.is_active(id))
        {
            auto* const state_ptr = ctx.active_state<internal_button_state>(id);

            if (state.hot)
            {
                auto delay_pref = ::LONG{};
                auto speed_pref = ::LONG{};
                ::SystemParametersInfoW(
                    SPI_GETKEYBOARDDELAY, 0, &delay_pref, 0);
                ::SystemParametersInfoW(
                    SPI_GETKEYBOARDSPEED, 0, &speed_pref, 0);

                auto const repeat_delay = 250ULL * delay_pref + 250;
                auto const repeat_speed = 12400ULL / (31 + 11ULL * speed_pref);

                auto const hold_time = mouse.hold_time(state_ptr->button);
                auto const hold_count =
                    (std::max(hold_time, repeat_delay) - repeat_delay) /
                    repeat_speed;

                if (hold_count != state_ptr->last_hold_count)
                {
                    state.pressed      = true;
                    state.repeat_count = state_ptr->repeat_count;
                    ++state_ptr->repeat_count;
                }
                state_ptr->last_hold_count = hold_count;
            }

            state.active        = true;
            state.button        = state_ptr->button;
            state.drag_offset   = mouse_position - state_ptr->drag_start;
            state.drag_position = state_ptr->drag_position + state.drag_offset;
            if (mouse.was_released(state.button))
            {
                state.pressed = false;
                ctx.deactivate(id);
                if (state.hot)
                {
                    state.clicked = true;
                }
            }
        }
    }
    else
    {
        ctx.deactivate(id);
    }

    return state;
}

}