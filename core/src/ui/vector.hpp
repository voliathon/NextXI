#ifndef WINDOWER_UI_VECTOR_HPP
#define WINDOWER_UI_VECTOR_HPP

#include <gsl/gsl>

#include <cmath>
#include <compare>

namespace windower::ui
{

struct vector
{
    float x = 0.f;
    float y = 0.f;

    constexpr vector() noexcept = default;
    constexpr vector(float x, float y) noexcept : x{x}, y{y} {}

    constexpr bool operator==(vector const&) const noexcept = default;
};

constexpr vector operator+(vector const& value) noexcept
{
    return {+value.x, +value.y};
}

constexpr vector operator-(vector const& value) noexcept
{
    return {-value.x, -value.y};
}

constexpr vector operator+(vector const& lhs, vector const& rhs) noexcept
{
    return {lhs.x + rhs.x, lhs.y + rhs.y};
}

constexpr vector operator-(vector const& lhs, vector const& rhs) noexcept
{
    return {lhs.x - rhs.x, lhs.y - rhs.y};
}

constexpr vector operator*(vector const& lhs, vector const& rhs) noexcept
{
    return {lhs.x * rhs.x, lhs.y * rhs.y};
}

constexpr vector operator*(vector const& lhs, float rhs) noexcept
{
    return {lhs.x * rhs, lhs.y * rhs};
}

constexpr vector operator*(float lhs, vector const& rhs) noexcept
{
    return {lhs * rhs.x, lhs * rhs.y};
}

constexpr vector operator/(vector const& lhs, vector const& rhs) noexcept
{
    return {lhs.x / rhs.x, lhs.y / rhs.y};
}

constexpr vector operator/(vector const& lhs, float rhs) noexcept
{
    return {lhs.x / rhs, lhs.y / rhs};
}

constexpr vector operator/(float lhs, vector const& rhs) noexcept
{
    return {lhs / rhs.x, lhs / rhs.y};
}

constexpr vector& operator+=(vector& lhs, vector const& rhs) noexcept
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    return lhs;
}

constexpr vector& operator-=(vector& lhs, vector const& rhs) noexcept
{
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    return lhs;
}

constexpr vector& operator*=(vector& lhs, float rhs) noexcept
{
    lhs.x *= rhs;
    lhs.y *= rhs;
    return lhs;
}

constexpr vector& operator/=(vector& lhs, float rhs) noexcept
{
    lhs.x /= rhs;
    lhs.y /= rhs;
    return lhs;
}

}

#endif