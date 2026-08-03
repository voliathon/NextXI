#include "guid.hpp"

#include <guiddef.h>
#include <objbase.h>

#include <gsl/gsl>

#include <algorithm>
#include <array>
#include <ranges>
#include <string>

windower::guid windower::guid::generate() noexcept
{
    auto guid = ::GUID{};
    if (SUCCEEDED(::CoCreateGuid(&guid)))
    {
        return guid;
    }
    return {};
}

std::span<std::byte const> windower::guid::raw() const noexcept
{
    return std::as_bytes(std::span{&m_guid, 1});
}

std::u8string windower::guid::string() const noexcept
{
    namespace range = std::ranges;
    namespace view  = std::ranges::views;

    std::array<::OLECHAR, 39> buffer{};
    if (::StringFromGUID2(m_guid, buffer.data(), buffer.size()) != 39)
    {
        return u8"00000000-0000-0000-0000-000000000000";
    }

    std::u8string result;
    range::transform(
        buffer | view::drop(1) | view::take(36), std::back_inserter(result),
        [](auto c) { return gsl::narrow_cast<char8_t>(c); });
    return result;
}