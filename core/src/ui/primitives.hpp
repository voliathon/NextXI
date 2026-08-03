#ifndef WINDOWER_UI_PRIMITIVES_HPP
#define WINDOWER_UI_PRIMITIVES_HPP

#include "ui/color.hpp"
#include "ui/context.hpp"
#include "ui/ffxi_image.hpp"
#include "ui/patch.hpp"
#include "ui/rectangle.hpp"
#include "ui/text_layout_engine.hpp"
#include "ui/texture_token.hpp"
#include "ui/vector.hpp"

#include <array>
#include <span>

namespace windower::ui
{

class context;

namespace primitive
{

void set_texture(
    context& ctx, std::u8string_view name,
    std::size_t time_to_live = 1) noexcept;

void set_texture(
    context& ctx, ffxi_image const& image,
    std::size_t time_to_live = 1) noexcept;

void set_texture(context& ctx, texture_token texture) noexcept;

texture_token get_texture(
    context& ctx, std::u8string_view name,
    std::size_t time_to_live = 1) noexcept;

texture_token get_texture(
    context& ctx, ffxi_image const& image,
    std::size_t time_to_live = 1) noexcept;

void rectangle(
    context& ctx, ui::rectangle const& bounds,
    color color = colors::white) noexcept;

void rectangle(
    context& ctx, ui::rectangle const& bounds, patch const& patch,
    color color = colors::white) noexcept;

void rectangle(
    context& ctx, ui::rectangle const& bounds, nine_patch const& patch,
    color color = colors::white) noexcept;

void rectangle(
    context& ctx, ui::rectangle const& bounds, h_patch const& patch,
    color color = colors::white) noexcept;

void rectangle(
    context& ctx, ui::rectangle const& bounds, v_patch const& patch,
    color color = colors::white) noexcept;

void text(
    context& ctx, ui::rectangle const& bounds, std::u8string_view text,
    text_rasterization_options const& options = {},
    text_layout_options const& layout_options = {}) noexcept;

void text(
    context& ctx, vector const& position, text_layout const& layout,
    text_rasterization_options const& rasterizeration_options = {}) noexcept;

text_layout layout_text(
    context& ctx, dimension const& size, std::u8string_view text,
    text_layout_options const& layout_options = {}) noexcept;

}

}

#endif