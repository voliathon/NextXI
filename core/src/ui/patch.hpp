#ifndef WINDOWER_UI_PATCH_HPP
#define WINDOWER_UI_PATCH_HPP

#include "ui/dimension.hpp"
#include "ui/rectangle.hpp"
#include "ui/thickness.hpp"

namespace windower::ui
{

class patch
{
public:
    dimension texture_size;
    rectangle bounds;
    thickness overdraw;

    constexpr patch() noexcept = default;

    constexpr patch(
        rectangle const& bounds, thickness const& overdraw = {1, 1, 1, 1},
        dimension const& texture_size = {256, 256}) noexcept :
        texture_size{texture_size},
        bounds{bounds}, overdraw{overdraw}
    {}
};

class nine_patch
{
public:
    dimension texture_size;
    rectangle bounds;
    thickness slice;
    thickness overdraw;

    constexpr nine_patch() noexcept = default;

    constexpr nine_patch(
        rectangle const& bounds, thickness const& slice,
        thickness const& overdraw     = {1, 1, 1, 1},
        dimension const& texture_size = {256, 256}) noexcept :
        texture_size{texture_size},
        bounds{bounds}, slice{slice}, overdraw{overdraw}
    {}
};

class h_patch
{
public:
    dimension texture_size;
    rectangle bounds;
    h_thickness slice;
    thickness overdraw;

    constexpr h_patch() noexcept = default;

    constexpr h_patch(
        rectangle const& bounds, h_thickness const& slice,
        thickness const& overdraw     = {1, 1, 1, 1},
        dimension const& texture_size = {256, 256}) noexcept :
        texture_size{texture_size},
        bounds{bounds}, slice{slice}, overdraw{overdraw}
    {}
};

class v_patch
{
public:
    dimension texture_size;
    rectangle bounds;
    v_thickness slice;
    thickness overdraw;

    constexpr v_patch() noexcept = default;

    constexpr v_patch(
        rectangle const& bounds, v_thickness const& slice,
        thickness const& overdraw     = {1, 1, 1, 1},
        dimension const& texture_size = {256, 256}) noexcept :
        texture_size{texture_size},
        bounds{bounds}, slice{slice}, overdraw{overdraw}
    {}
};

}

#endif