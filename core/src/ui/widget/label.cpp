#include "ui/widget/label.hpp"

#include "ui/context.hpp"
#include "ui/primitives.hpp"

namespace windower::ui::widget
{

void label(context& ctx, std::u8string_view text) noexcept
{
    primitive::text(
        ctx, expand(ctx.bounds(), {0, 3}), text,
        {
            .fill_color = ctx.enabled()
                              ? ctx.system_color(system_color::label)
                              : ctx.system_color(system_color::label_disabled),
        });
}

}