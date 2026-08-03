#ifndef WINDOWER_UI_VERTEX_HPP
#define WINDOWER_UI_VERTEX_HPP

#include "ui/color.hpp"

namespace windower::ui
{

class vertex
{
public:
    constexpr vertex() noexcept = default;

    constexpr vertex(float x, float y, color color = colors::white) noexcept :
        x{x}, y{y}, color{color}
    {}

    constexpr vertex(
        float x, float y, float z, float rhw, float u, float v,
        color color = colors::white) noexcept :
        x{x},
        y{y}, z{z}, rhw{rhw}, color{color}, u{u}, v{v}
    {}

    float x{0.f};
    float y{0.f};
    float z{0.f};
    float rhw{10.f};
    ui::color color{colors::white};
    float u{0.f};
    float v{0.f};
};

}

#endif