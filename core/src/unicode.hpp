#ifndef WINDOWER_UNICODE_BASE_HPP
#define WINDOWER_UNICODE_BASE_HPP

#include <gsl/gsl>

#include <string>
#include <string_view>

namespace windower
{

enum class client_language : std::uint8_t
{
    japanese = 1,
    english  = 2,
};

using sjis_char = unsigned char;

using sjis_string      = std::basic_string<sjis_char>;
using sjis_string_view = std::basic_string_view<sjis_char>;

std::u8string to_u8string(std::wstring_view str) noexcept;
std::u8string to_u8string(sjis_string_view str) noexcept;
std::wstring to_wstring(std::u8string_view str) noexcept;
std::wstring to_wstring(sjis_string_view str) noexcept;
sjis_string to_sjis_string(
    std::u8string_view str,
    client_language client_language = client_language::english) noexcept;
sjis_string to_sjis_string(
    std::wstring_view str,
    client_language client_language = client_language::english) noexcept;

char32_t next_code_point(std::u8string_view str, std::size_t& offset) noexcept;
char32_t next_code_point(std::wstring_view str, std::size_t& offset) noexcept;
char32_t next_code_point(sjis_string_view str, std::size_t& offset) noexcept;

void append(std::u8string& str, char32_t code_point) noexcept;
void append(std::wstring& str, char32_t code_point) noexcept;
void append(
    sjis_string& str, char32_t code_point,
    client_language client_language = client_language::english) noexcept;

std::u8string nfkc_fold_case(std::u8string_view) noexcept;

std::uint32_t to_autotranslate_id(
    char32_t code_point,
    client_language client_language = client_language::english) noexcept;

bool is_whitespace(char32_t code_point) noexcept;

constexpr bool is_autotranslate(char32_t code_point) noexcept
{
    return code_point >= U'\U000F0000' && code_point <= U'\U000FFFFD' ||
           code_point >= U'\U00100000' && code_point <= U'\U0010FFFD';
}

}

#endif