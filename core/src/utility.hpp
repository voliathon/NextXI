#ifndef WINDOWER_UTILITY_HPP
#define WINDOWER_UTILITY_HPP

#include <gsl/gsl>

#include <algorithm>
#include <bit>
#include <cstddef>
#include <filesystem>
#include <numeric>
#include <span>
#include <string>
#include <type_traits>

#include "utilities/string_helpers.hpp"

#if defined(_MSC_VER)
#    include <intrin.h>
#    define WINDOWER_DEBUG_BREAK __debugbreak()
#    define WINDOWER_RETURN_ADDRESS _ReturnAddress()
#elif defined(__GNUC__)
#    define WINDOWER_DEBUG_BREAK __builtin_trap()
#    define WINDOWER_RETURN_ADDRESS                                            \
        __builtin_extract_return_addr(__builtin_return_address(0))
#else
#    include <windows.h>
#    define WINDOWER_DEBUG_BREAK ::DebugBreak()
#    error "WINDOWER_RETURN_ADDRESS macro is not defined for this compiler"
#endif

#if defined(__clang__)
#    define WINDOWER_SUPPRESS(id) [[gsl::suppress(#id)]]
#elif defined(_MSC_VER)
#    define WINDOWER_SUPPRESS(id) [[gsl::suppress(id)]]
#endif

namespace windower
{

template<typename T>
constexpr std::underlying_type_t<T> to_underlying(T value)
{
    return gsl::narrow_cast<std::underlying_type_t<T>>(value);
}

template<typename T>
constexpr bool has_flag(T value, T flag) noexcept
{
    return (to_underlying(value) & to_underlying(flag)) != 0;
}

template<typename T>
T map_value(
    T const& value, T const& in_min, T const& in_max, T const& out_min,
    T const& out_max) noexcept
{
    if (in_min == in_max)
    {
        return std::midpoint(out_min, out_max);
    }
    auto const in_range  = in_max - in_min;
    auto const out_range = out_max - out_min;
    return (value - in_min) * out_range / in_range + out_min;
}

template<typename C, typename T = std::char_traits<C>>
class basic_format_guard
{
public:
    basic_format_guard(basic_format_guard const&) = delete;
    basic_format_guard(basic_format_guard&&)      = delete;
    basic_format_guard(std::basic_ios<C, T>& stream) :
        m_stream{stream}, m_format{nullptr}
    {
        m_format.copyfmt(m_stream);
    }

    ~basic_format_guard() { m_stream.copyfmt(m_format); }

    basic_format_guard& operator=(basic_format_guard const&) = delete;
    basic_format_guard& operator=(basic_format_guard&&) = delete;

private:
    std::ios& m_stream;
    std::ios m_format;
};

using format_guard = basic_format_guard<char>;

template<typename T, std::size_t N>
constexpr std::size_t array_size(T const (&)[N]) noexcept
{
    return N;
}

#ifdef _MSC_VER
template<typename T>
constexpr T change_endian(T value)
{
    if constexpr (std::is_integral_v<T> && sizeof(T) == sizeof(unsigned short))
    {
        return std::bit_cast<T>(
            _byteswap_ushort(std::bit_cast<unsigned short>(value)));
    }
    else if constexpr (
        std::is_integral_v<T> && sizeof(T) == sizeof(unsigned long))
    {
        return std::bit_cast<T>(
            _byteswap_ulong(std::bit_cast<unsigned long>(value)));
    }
    else if constexpr (std::is_integral_v<T> && sizeof(T) == sizeof(__int64))
    {
        return std::bit_cast<T>(
            _byteswap_uint64(std::bit_cast<__int64>(value)));
    }
}
#endif

template<typename... T>
class overloaded : public T...
{
public:
    using T::operator()...;
};

template<typename... T>
overloaded(T...) -> overloaded<T...>;

}

#endif
