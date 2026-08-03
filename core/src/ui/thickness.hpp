#ifndef WINDOWER_UI_THICKNESS_HPP
#define WINDOWER_UI_THICKNESS_HPP

namespace windower::ui
{

class thickness
{
public:
    float left   = 0.f;
    float top    = 0.f;
    float right  = 0.f;
    float bottom = 0.f;

    constexpr thickness() noexcept = default;

    constexpr thickness(float uniform) noexcept :
        top{uniform}, right{uniform}, bottom{uniform}, left{uniform}
    {}

    constexpr thickness(float left_right, float top_bottom) noexcept :
        left{left_right}, top{top_bottom}, right{left_right}, bottom{top_bottom}
    {}

    constexpr thickness(float left, float top_bottom, float right) noexcept :
        left{left}, top{top_bottom}, right{right}, bottom{top_bottom}
    {}

    constexpr thickness(
        float left, float top, float right, float bottom) noexcept :
        left{left},
        top{top}, right{right}, bottom{bottom}
    {}
};

class h_thickness
{
public:
    float left  = 0.f;
    float right = 0.f;

    constexpr h_thickness() noexcept = default;

    constexpr h_thickness(float uniform) noexcept :
        left{uniform}, right{uniform}
    {}

    constexpr h_thickness(float left, float right) noexcept :
        left{left}, right{right}
    {}

    constexpr operator thickness() const noexcept
    {
        return {left, 0, right, 0};
    }
};

class v_thickness
{
public:
    float top    = 0.f;
    float bottom = 0.f;

    constexpr v_thickness() noexcept = default;

    constexpr v_thickness(float uniform) noexcept :
        top{uniform}, bottom{uniform}
    {}

    constexpr v_thickness(float top, float bottom) noexcept :
        top{top}, bottom{bottom}
    {}

    constexpr operator thickness() const noexcept
    {
        return {0, top, 0, bottom};
    }
};

}

#endif