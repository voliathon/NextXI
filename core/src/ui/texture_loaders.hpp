#ifndef WINDOWER_UI_TEXTURE_LOADERS_HPP
#define WINDOWER_UI_TEXTURE_LOADERS_HPP

#include "ui/context.hpp"
#include "ui/ffxi_image.hpp"
#include "ui/texture.hpp"

#include <cstddef>
#include <string_view>

namespace windower::ui
{

texture const& load_texture(
    context& ctx, std::u8string_view name, std::size_t time_to_live) noexcept;

texture const& load_texture(
    context& ctx, ffxi_image const& image, std::size_t time_to_live) noexcept;

}

#endif