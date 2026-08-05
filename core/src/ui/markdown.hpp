#ifndef WINDOWER_UI_MARKDOWN_HPP
#define WINDOWER_UI_MARKDOWN_HPP

#include "ui/color.hpp"

#include <dwrite.h>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace windower::ui
{

enum class size_unit
{
    absolute,
    relative,
    pixel,
    point,
    pica,
};

struct format
{};

struct typeface
{
    std::wstring value;
};

struct font_size
{
    size_unit unit;
    float value;
};

struct stretch
{
    ::DWRITE_FONT_STRETCH value;
};

struct weight
{
    ::DWRITE_FONT_WEIGHT value;
};

struct style
{
    ::DWRITE_FONT_STYLE value;
};

struct strikethrough
{
    bool value;
};

struct underline
{
    bool value;
};

struct stroke
{
    std::optional<font_size> width;
    std::optional<color> color;
};

using option = std::variant<
    format, typeface, font_size, stretch, weight, style, strikethrough,
    underline, color, stroke>;

struct fragment
{
    std::size_t start;
    std::size_t length;
    option data;
};

std::vector<fragment> parse_markdown(std::wstring_view text) noexcept;

}

#endif