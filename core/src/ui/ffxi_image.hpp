#ifndef WINDOWER_UI_FFXI_IMAGE_HPP
#define WINDOWER_UI_FFXI_IMAGE_HPP

#include "ui/dimension.hpp"
#include "ui/patch.hpp"

#include <windows.h>

#include <d3d8.h>

#include <gsl/gsl>

#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>

namespace windower::ui
{

#pragma pack(push, 1)
class ffxi_image
{
public:
    ::D3DFORMAT d3d_format() const noexcept;

    ui::patch patch() const noexcept;
    ui::dimension size() const noexcept;
    ui::dimension raw_size() const noexcept;

    bool copy_to(::IDirect3DTexture8*) const noexcept;

private:
    std::uint8_t m_type;
    char8_t m_name[16];
    ::BITMAPINFOHEADER m_bitmap_header;
    std::byte m_data[1];

    std::span<std::byte const>
    raw_data(std::size_t offset, std::size_t size) const noexcept;

    void copy_indexed_to(std::span<std::byte>, std::int32_t) const noexcept;
    void copy_rgba_to(std::span<std::byte>, std::int32_t) const noexcept;
    void copy_dxt_to(std::span<std::byte>, std::int32_t) const noexcept;
};
#pragma pack(pop)

static_assert(std::is_standard_layout_v<ffxi_image>);
static_assert(sizeof(ffxi_image) == 58);

}

#endif