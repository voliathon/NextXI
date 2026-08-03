#include "ui/widget/error_box.hpp"

#include "ui/context.hpp"
#include "ui/id.hpp"
#include "ui/primitives.hpp"
#include "ui/rectangle.hpp"
#include "ui/text_rasterization_options.hpp"

#include <charconv>
#include <string_view>

namespace windower::ui::widget
{
namespace
{

constexpr auto box = nine_patch{{11, 191, 73, 253}, {5}};

}

void error_box(
    context& ctx, id id, std::u8string_view widget,
    std::u8string_view message) noexcept
{
    auto const& bounds = ctx.bounds();

    primitive::set_texture(ctx, u8":system");
    primitive::rectangle(ctx, bounds, box);

    auto raw_buffer = std::array<char, 19>{u8'['};
    auto raw_span   = std::span<char>{raw_buffer}.subspan(1);

    auto const id_value = id.value();
    auto const id_scope = id.scope();

    if (auto const result = std::to_chars(
            std::to_address(raw_span.begin()), std::to_address(raw_span.end()),
            id_value, 16);
        result.ec == std::errc{} &&
        result.ptr != std::to_address(raw_span.end()))
    {
        *result.ptr = u8':';
        raw_span    = raw_span.subspan(result.ptr - raw_span.data() + 1);
    }

    if (auto const result = std::to_chars(
            std::to_address(raw_span.begin()), std::to_address(raw_span.end()),
            id_scope, 16);
        result.ec == std::errc{} &&
        result.ptr != std::to_address(raw_span.end()))
    {
        *result.ptr = u8':';
        raw_span    = raw_span.subspan(result.ptr - raw_span.data() + 1);
    }

    auto buffer     = std::array<char8_t, 20>{};
    auto const size = raw_span.data() - raw_buffer.data();
    auto const span = std::span{buffer}.subspan(0, size);
    std::copy_n(raw_buffer.begin(), span.size(), span.begin());
    auto const id_str = std::u8string_view{span.data(), span.size()};

    auto const x = bounds.x0 + 6;
    auto y       = bounds.y0 + 4;

    auto const id_layout = primitive::layout_text(
        ctx, {bounds.width() - 12, bounds.height() - 8}, id_str,
        {.word_wrapping = text_word_wrapping::no_wrap});
    primitive::text(
        ctx, {x, y}, id_layout,
        {.fill_color   = ctx.system_color(system_color::white),
         .stroke_color = ctx.system_color(system_color::error),
         .stroke_width = 1});
    y += id_layout.metric_bounds().height();

    auto const widget_layout = primitive::layout_text(
        ctx, {bounds.width() - 12, bounds.height() - y - 4}, widget,
        {.word_wrapping = text_word_wrapping::no_wrap});
    primitive::text(
        ctx, {x, y}, widget_layout,
        {.fill_color   = ctx.system_color(system_color::white),
         .stroke_color = ctx.system_color(system_color::error),
         .stroke_width = 1});
    y += id_layout.metric_bounds().height();

    primitive::text(
        ctx, {x, y, x + bounds.width() - 12, bounds.height() - 4}, message,
        {.fill_color   = ctx.system_color(system_color::white),
         .stroke_color = ctx.system_color(system_color::error),
         .stroke_width = 1});
}

}