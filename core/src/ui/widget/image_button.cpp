#include "ui/widget/image_button.hpp"

#include "ui/context.hpp"
#include "ui/id.hpp"
#include "ui/primitives.hpp"
#include "ui/widget/basic_button.hpp"

namespace windower::ui::widget
{

button_state image_button(
    context& ctx, id id, image_button_descriptor const& descriptor) noexcept
{
    auto const state = basic_button(ctx, id);

    primitive::set_texture(ctx, descriptor.image());
    if (!ctx.enabled())
    {
        primitive::rectangle(ctx, ctx.bounds(), descriptor.disabled());
    }
    else if (state.active && state.button == mouse_button::left)
    {
        ctx.set_cursor(descriptor.cursor());
        if (state.hot)
        {
            primitive::rectangle(ctx, ctx.bounds(), descriptor.active());
        }
        else
        {
            primitive::rectangle(ctx, ctx.bounds(), descriptor.hot());
        }
    }
    else if (state.hot)
    {
        ctx.set_cursor(descriptor.cursor());
        primitive::rectangle(ctx, ctx.bounds(), descriptor.hot());
    }
    else
    {
        primitive::rectangle(ctx, ctx.bounds(), descriptor.normal());
    }

    return state;
}

}