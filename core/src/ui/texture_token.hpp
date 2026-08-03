#ifndef WINDOWER_UI_TEXTURE_TOKEN_HPP
#define WINDOWER_UI_TEXTURE_TOKEN_HPP

#include "ui/patch.hpp"

#include <d3d8.h>

namespace windower::ui
{

class texture_token
{
public:
    constexpr texture_token() = default;
    constexpr texture_token(::IDirect3DTexture8* value) : m_value{value} {}

    constexpr bool operator==(texture_token const& other) const = default;

private:
    ::IDirect3DTexture8* m_value = nullptr;

    friend class context;
    friend class texture_cache;
};

constexpr texture_token no_texture;

}

#endif