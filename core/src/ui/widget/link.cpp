#include "ui/widget/link.hpp"

#include "ui/context.hpp"
#include "ui/id.hpp"
#include "ui/primitives.hpp"
#include "ui/widget/basic_button.hpp"

namespace windower::ui::widget
{

button_state link(context& ctx, id id, std::u8string_view text) noexcept
{
    auto const state = basic_button(ctx, id);

    if (state.hot || state.active && state.button == mouse_button::left)
    {
        ctx.set_cursor(system_cursor::hot);
    }

    primitive::text(
        ctx, expand(ctx.bounds(), {0, 3}), text,
        {.fill_color = ctx.enabled()
                           ? ctx.system_color(system_color::link)
                           : ctx.system_color(system_color::link_disabled)},
        {.underline = true});

    return state;
}

}