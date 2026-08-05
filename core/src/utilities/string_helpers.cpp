#include "string_helpers.hpp"

#include <gsl/gsl>
#include <array>
#include <charconv>

std::string_view windower::to_string_view(std::u8string_view value) noexcept
{
    static_assert(char_is_ascii());

    [[gsl::suppress("type.1")]]
    {
        return { reinterpret_cast<char const*>(value.data()), value.size() };
    }
}

windower::zstring_view windower::to_zstring_view(u8zstring_view value) noexcept
{
    static_assert(char_is_ascii());

    [[gsl::suppress("type.1")]]
    {
        return { reinterpret_cast<char const*>(value.data()), value.size() };
    }
}

std::u8string windower::to_u8string(std::string_view value)
{
    static_assert(char_is_ascii());
    return { value.begin(), value.end() };
}

std::u8string windower::to_u8string(signed char value, int base)
{
    static_assert(char_is_ascii());
    static constexpr auto digits = std::numeric_limits<signed char>::digits10 + 1;
    auto buffer = std::array<char, digits>{};
    auto const begin = buffer.data();
    auto const end = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value, base);
    return { begin, result.ptr };
}

std::u8string windower::to_u8string(signed short int value, int base)
{
    static_assert(char_is_ascii());
    static constexpr auto digits = std::numeric_limits<signed short int>::digits10 + 1;
    auto buffer = std::array<char, digits>{};
    auto const begin = buffer.data();
    auto const end = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value, base);
    return { begin, result.ptr };
}

std::u8string windower::to_u8string(signed int value, int base)
{
    static_assert(char_is_ascii());
    static constexpr auto digits = std::numeric_limits<signed int>::digits10 + 1;
    auto buffer = std::array<char, digits>{};
    auto const begin = buffer.data();
    auto const end = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value, base);
    return { begin, result.ptr };
}

std::u8string windower::to_u8string(signed long int value, int base)
{
    static_assert(char_is_ascii());
    static constexpr auto digits = std::numeric_limits<signed long int>::digits10 + 1;
    auto buffer = std::array<char, digits>{};
    auto const begin = buffer.data();
    auto const end = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value, base);
    return { begin, result.ptr };
}

std::u8string windower::to_u8string(signed long long int value, int base)
{
    static_assert(char_is_ascii());
    static constexpr auto digits = std::numeric_limits<signed long long int>::digits10 + 1;
    auto buffer = std::array<char, digits>{};
    auto const begin = buffer.data();
    auto const end = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value, base);
    return { begin, result.ptr };
}

std::u8string windower::to_u8string(unsigned char value, int base)
{
    static_assert(char_is_ascii());
    static constexpr auto digits = std::numeric_limits<unsigned char>::digits10;
    auto buffer = std::array<char, digits>{};
    auto const begin = buffer.data();
    auto const end = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value, base);
    return { begin, result.ptr };
}

std::u8string windower::to_u8string(unsigned short int value, int base)
{
    static_assert(char_is_ascii());
    static constexpr auto digits = std::numeric_limits<unsigned short int>::digits10;
    auto buffer = std::array<char, digits>{};
    auto const begin = buffer.data();
    auto const end = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value, base);
    return { begin, result.ptr };
}

std::u8string windower::to_u8string(unsigned int value, int base)
{
    static_assert(char_is_ascii());
    static constexpr auto digits = std::numeric_limits<unsigned int>::digits10;
    auto buffer = std::array<char, digits>{};
    auto const begin = buffer.data();
    auto const end = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value, base);
    return { begin, result.ptr };
}

std::u8string windower::to_u8string(unsigned long int value, int base)
{
    static_assert(char_is_ascii());
    static constexpr auto digits = std::numeric_limits<unsigned long int>::digits10;
    auto buffer = std::array<char, digits>{};
    auto const begin = buffer.data();
    auto const end = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value, base);
    return { begin, result.ptr };
}

std::u8string windower::to_u8string(unsigned long long int value, int base)
{
    static_assert(char_is_ascii());
    static constexpr auto digits = std::numeric_limits<unsigned long long int>::digits10;
    auto buffer = std::array<char, digits>{};
    auto const begin = buffer.data();
    auto const end = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value, base);
    return { begin, result.ptr };
}

std::u8string windower::to_u8string(float value)
{
    static_assert(char_is_ascii());
    static constexpr auto digits = std::numeric_limits<float>::max_digits10;
    auto buffer = std::array<char, digits>{};
    auto const begin = buffer.data();
    auto const end = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value);
    return { begin, result.ptr };
}

std::u8string windower::to_u8string(double value)
{
    static_assert(char_is_ascii());
    static constexpr auto digits = std::numeric_limits<double>::max_digits10;
    auto buffer = std::array<char, digits>{};
    auto const begin = buffer.data();
    auto const end = std::next(begin, buffer.size());
    auto const result = std::to_chars(begin, end, value);
    return { begin, result.ptr };
}

std::string windower::to_string(std::u8string_view value)
{
    static_assert(char_is_ascii());
    return { value.begin(), value.end() };
}

std::size_t windower::parse(std::u8string_view s, std::int8_t& value, int base) noexcept { return parse(to_string_view(s), value, base); }
std::size_t windower::parse(std::u8string_view s, std::int16_t& value, int base) noexcept { return parse(to_string_view(s), value, base); }
std::size_t windower::parse(std::u8string_view s, std::int32_t& value, int base) noexcept { return parse(to_string_view(s), value, base); }
std::size_t windower::parse(std::u8string_view s, std::int64_t& value, int base) noexcept { return parse(to_string_view(s), value, base); }
std::size_t windower::parse(std::u8string_view s, std::uint8_t& value, int base) noexcept { return parse(to_string_view(s), value, base); }
std::size_t windower::parse(std::u8string_view s, std::uint16_t& value, int base) noexcept { return parse(to_string_view(s), value, base); }
std::size_t windower::parse(std::u8string_view s, std::uint32_t& value, int base) noexcept { return parse(to_string_view(s), value, base); }
std::size_t windower::parse(std::u8string_view s, std::uint64_t& value, int base) noexcept { return parse(to_string_view(s), value, base); }

std::size_t windower::parse(std::string_view s, std::int8_t& value, int base) noexcept
{
    auto const begin = s.data();
    auto const end = std::next(begin, s.size());
    auto const result = std::from_chars(begin, end, value, base);
    return result.ec != std::errc::result_out_of_range ? gsl::narrow_cast<std::size_t>(result.ptr - s.data()) : 0;
}

std::size_t windower::parse(std::string_view s, std::int16_t& value, int base) noexcept
{
    auto const begin = s.data();
    auto const end = std::next(begin, s.size());
    auto const result = std::from_chars(begin, end, value, base);
    return result.ec != std::errc::result_out_of_range ? gsl::narrow_cast<std::size_t>(result.ptr - s.data()) : 0;
}

std::size_t windower::parse(std::string_view s, std::int32_t& value, int base) noexcept
{
    auto const begin = s.data();
    auto const end = std::next(begin, s.size());
    auto const result = std::from_chars(begin, end, value, base);
    return result.ec != std::errc::result_out_of_range ? gsl::narrow_cast<std::size_t>(result.ptr - s.data()) : 0;
}

std::size_t windower::parse(std::string_view s, std::int64_t& value, int base) noexcept
{
    auto const begin = s.data();
    auto const end = std::next(begin, s.size());
    auto const result = std::from_chars(begin, end, value, base);
    return result.ec != std::errc::result_out_of_range ? gsl::narrow_cast<std::size_t>(result.ptr - s.data()) : 0;
}

std::size_t windower::parse(std::string_view s, std::uint8_t& value, int base) noexcept
{
    auto const begin = s.data();
    auto const end = std::next(begin, s.size());
    auto const result = std::from_chars(begin, end, value, base);
    return result.ec != std::errc::result_out_of_range ? gsl::narrow_cast<std::size_t>(result.ptr - s.data()) : 0;
}

std::size_t windower::parse(std::string_view s, std::uint16_t& value, int base) noexcept
{
    auto const begin = s.data();
    auto const end = std::next(begin, s.size());
    auto const result = std::from_chars(begin, end, value, base);
    return result.ec != std::errc::result_out_of_range ? gsl::narrow_cast<std::size_t>(result.ptr - s.data()) : 0;
}

std::size_t windower::parse(std::string_view s, std::uint32_t& value, int base) noexcept
{
    auto const begin = s.data();
    auto const end = std::next(begin, s.size());
    auto const result = std::from_chars(begin, end, value, base);
    return result.ec != std::errc::result_out_of_range ? gsl::narrow_cast<std::size_t>(result.ptr - s.data()) : 0;
}

std::size_t windower::parse(std::string_view s, std::uint64_t& value, int base) noexcept
{
    auto const begin = s.data();
    auto const end = std::next(begin, s.size());
    auto const result = std::from_chars(begin, end, value, base);
    return result.ec != std::errc::result_out_of_range ? gsl::narrow_cast<std::size_t>(result.ptr - s.data()) : 0;
}
