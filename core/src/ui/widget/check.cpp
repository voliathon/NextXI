#include "ui/widget/check.hpp"

#include "ui/context.hpp"
#include "ui/dimension.hpp"
#include "ui/id.hpp"
#include "ui/patch.hpp"
#include "ui/primitives.hpp"
#include "ui/text_layout_engine.hpp"
#include "ui/widget/basic_button.hpp"

namespace windower::ui::widget
{
namespace
{

constexpr auto off_normal   = patch{{11, 79, 25, 93}};
constexpr auto off_hot      = patch{{27, 79, 41, 93}};
constexpr auto off_active   = patch{{43, 79, 57, 93}};
constexpr auto off_disabled = patch{{59, 79, 73, 93}};

constexpr auto on_normal   = patch{{11, 95, 25, 109}};
constexpr auto on_hot      = patch{{27, 95, 41, 109}};
constexpr auto on_active   = patch{{43, 95, 57, 109}};
constexpr auto on_disabled = patch{{59, 95, 73, 109}};

constexpr auto tri_normal   = patch{{11, 111, 25, 125}};
constexpr auto tri_hot      = patch{{27, 111, 41, 125}};
constexpr auto tri_active   = patch{{43, 111, 57, 125}};
constexpr auto tri_disabled = patch{{59, 111, 73, 125}};

}

button_state check(
    context& ctx, id id, std::u8string_view text,
    std::optional<bool> checked) noexcept
{
    auto const enabled         = ctx.enabled();
    auto const original_bounds = ctx.bounds();
    auto const position        = original_bounds.position();
    auto text_options =
        text_layout_options{.word_wrapping = text_word_wrapping::no_wrap};
    auto const text_layout = primitive::layout_text(
        ctx, {dimension::unbounded, dimension::unbounded}, text, text_options);

    float const text_width = std::ceil(text_layout.metric_bounds().width());
    float const total_active_width = 29.f + text_width;
    auto const hit_bounds = rectangle{
        original_bounds.x0, // Safely anchored to the left wall
        original_bounds.y0 - 6.f,
        original_bounds.x0 + total_active_width, // Clamp the highlight!
        original_bounds.y1 - 6.f};

    ctx.bounds(hit_bounds);
    auto const state = basic_button(ctx, id);
    ctx.bounds(original_bounds);
    auto const check_bounds = rectangle{
        position.x + 5.f, // Shift box right
        position.y + 2.f,
        position.x + 19.f, // 5 + 14 = 19
        position.y + 16.f};
    
    auto text_opts = text_rasterization_options{};
    auto const current_style = ctx.current_style();
    if (current_style.text_color)
    {
        text_opts.fill_color = *current_style.text_color;
    }
    if (!enabled)
    {
        text_opts.fill_color = ctx.system_color(system_color::label_disabled);
    }
    primitive::text(
        ctx, {position.x + 25.f, position.y + 1.f}, text_layout, text_opts);

    primitive::set_texture(ctx, ctx.skin());

    if (!ctx.enabled())
    {
        primitive::rectangle(
            ctx, check_bounds,
            checked ? *checked ? on_disabled : off_disabled : tri_disabled);
    }
    else if (state.active && state.button == mouse_button::left)
    {
        ctx.set_cursor(system_cursor::hot);
        if (state.hot)
        {
            primitive::rectangle(
                ctx, check_bounds,
                checked ? *checked ? on_active : off_active : tri_active);
        }
        else
        {
            primitive::rectangle(
                ctx, check_bounds,
                checked ? *checked ? on_hot : off_hot : tri_hot);
        }
    }
    else if (state.hot)
    {
        ctx.set_cursor(system_cursor::hot);
        primitive::rectangle(
            ctx, check_bounds, checked ? *checked ? on_hot : off_hot : tri_hot);
    }
    else
    {
        primitive::rectangle(
            ctx, check_bounds,
            checked ? *checked ? on_normal : off_normal : tri_normal);
    }

    return state;
}

}