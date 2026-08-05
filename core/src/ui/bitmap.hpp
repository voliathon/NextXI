#ifndef WINDOWER_UI_BITMAP_HPP
#define WINDOWER_UI_BITMAP_HPP

#include "guid.hpp"
#include "ui/color.hpp"
#include "ui/ffxi_image.hpp"
#include "ui/patch.hpp"
#include "ui/vector.hpp"

#include <windows.h>

#include <d3d8.h>
#include <wincodec.h>
#include <winrt/base.h>

#include <cstddef>
#include <optional>
#include <span>
#include <variant>

namespace windower::ui
{

class context;

class bitmap
{
public:
    static bitmap load(context& ctx, std::u8string_view name) noexcept;

    bitmap() noexcept = default;
    bitmap(winrt::com_ptr<::IWICBitmapSource>, ui::patch const&) noexcept;

    explicit operator bool() const noexcept;

    guid wic_format() const noexcept;
    ::D3DFORMAT d3d_format() const noexcept;
    bitmap convert(context& ctx, ::WICPixelFormatGUID const&) const noexcept;
    bitmap convert(context& ctx) const noexcept;

    ::IWICBitmapSource* get() const noexcept;
    ui::patch const& patch() const noexcept;
    ui::dimension const& size() const noexcept;
    ui::dimension raw_size() const noexcept;

    color sample(context&, vector const&) const noexcept;
    bool copy_to(::IDirect3DTexture8*) const noexcept;

private:
    winrt::com_ptr<::IWICBitmapSource> m_bitmap;
    ui::patch m_patch;
};

}

#endif