#ifndef WINDOWER_GUID_HPP
#define WINDOWER_GUID_HPP

#include <guiddef.h>

#include <algorithm>
#include <cstddef>
#include <span>
#include <string>

namespace windower
{

class guid
{
public:
    static guid generate() noexcept;

    constexpr guid() noexcept = default;
    constexpr guid(::GUID const& guid) noexcept : m_guid{guid} {}

    constexpr bool operator==(guid const& other) const noexcept
    {
        return std::ranges::equal(raw(), other.raw());
    }

    constexpr ::GUID const& get() const noexcept { return m_guid; }
    constexpr ::GUID* put() noexcept { return &m_guid; }

    std::span<std::byte const> raw() const noexcept;
    std::u8string string() const noexcept;

private:
    ::GUID m_guid = {};
};

}

#endif