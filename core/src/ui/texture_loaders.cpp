#include "ui/texture_loaders.hpp"

#include "ui/bitmap.hpp"
#include "ui/context.hpp"
#include "ui/ffxi_image.hpp"
#include "ui/texture.hpp"
#include "utility.hpp"

#include <d3d8.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>
#include <utility>

namespace windower::ui
{

texture const& load_texture(
    context& ctx, std::u8string_view name, std::size_t time_to_live) noexcept
{
    auto desc = texture_cache::descriptor{texture_type::file, name};
    desc.set_float(0, ctx.scale_factor_uniform());
    return ctx.texture_cache().get(desc, time_to_live, [&]() noexcept {
        auto const bitmap = bitmap::load(ctx, name);
        if (auto const converted = bitmap.convert(ctx))
        {
            if (auto const format = converted.d3d_format();
                format != D3DFMT_UNKNOWN)
            {
                auto const size = converted.raw_size();
                if (auto tex = texture_cache::allocate(ctx, size, format);
                    tex && converted.copy_to(tex.get()))
                {
                    return texture{tex.detach(), converted.patch()};
                }
            }
        }
        return texture{};
    });
}

texture const& load_texture(
    context& ctx, ffxi_image const& image, std::size_t time_to_live) noexcept
{
    auto const desc = texture_cache::descriptor{&image};
    return ctx.texture_cache().get(desc, time_to_live, [&]() noexcept {
        auto const format = image.d3d_format();
        if (format != ::D3DFMT_UNKNOWN)
        {
            auto const size = image.raw_size();
            if (auto tex = texture_cache::allocate(ctx, size, format);
                tex && image.copy_to(tex.get()))
            {
                return texture{tex.detach(), image.patch()};
            }
        }
        return texture{};
    });
}

}