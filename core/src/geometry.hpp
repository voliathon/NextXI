#ifndef WINDOWER_GEOMETRY_HPP
#define WINDOWER_GEOMETRY_HPP

#include <algorithm>
#include <cstdint>

namespace windower
{
    struct point
    {
        std::int32_t x;
        std::int32_t y;
    };

    struct dimension
    {
        std::int32_t width;
        std::int32_t height;
    };

    struct rectangle
    {
        point location;
        dimension size;
    };

    constexpr rectangle intersect(rectangle lhs, rectangle rhs)
    {
        auto l = std::max(lhs.location.x, rhs.location.x);
        auto t = std::max(lhs.location.y, rhs.location.y);
        auto r = std::min(lhs.location.x + lhs.size.width, rhs.location.x + rhs.size.width);
        auto b = std::min(lhs.location.y + lhs.size.height, rhs.location.y + rhs.size.height);
        return {{l, t}, {r <= l ? 0 : r - l, b <= t ? 0 : b - t}};
    }
}

#endif