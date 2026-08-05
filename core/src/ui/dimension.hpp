#ifndef WINDOWER_UI_SIZE_HPP
#define WINDOWER_UI_SIZE_HPP

#include "ui/vector.hpp"

#include <limits>

namespace windower::ui
{

class dimension
{
public:
    static constexpr float unbounded = std::numeric_limits<float>::infinity();

    float width  = 0.f;
    float height = 0.f;

    constexpr dimension() noexcept = default;

    constexpr dimension(float width, float height) noexcept :
        width{width}, height{height}
    {}

    constexpr explicit dimension(vector const& vector) noexcept :
        dimension{vector.x, vector.y}
    {}

    constexpr bool operator==(dimension const&) const noexcept = default;

    constexpr explicit operator vector() const noexcept
    {
        return {width, height};
    }
};

constexpr std::strong_ordering
strong_order(dimension const& lhs, dimension const& rhs) noexcept
{
    if (auto const result = std::strong_order(lhs.width, rhs.width);
        result != std::strong_ordering::equal)
    {
        return result;
    }
    return std::strong_order(lhs.height, rhs.height);
}

}

#endif