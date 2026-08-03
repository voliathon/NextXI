#ifndef WINDOWER_UI_TEXT_LAYOUT_OPTIONS_HPP
#define WINDOWER_UI_TEXT_LAYOUT_OPTIONS_HPP

#include "ui/thickness.hpp"

#include <compare>
#include <cstdint>
#include <string>
#include <string_view>

namespace windower::ui
{

enum class text_alignment : std::uint8_t
{
    left      = 0,
    right     = 1,
    center    = 2,
    justified = 3,
};

enum class text_vertical_alignment : std::uint8_t
{
    top    = 0,
    bottom = 1,
    middle = 2,
};

enum class text_word_wrapping : std::uint8_t
{
    wrap            = 0,
    no_wrap         = 1,
    emergency_break = 2,
    whole_word      = 3,
    character       = 4,
};

enum class text_trimming_granularity : std::uint8_t
{
    none      = 0,
    character = 1,
    word      = 2,
};

class text_layout_options
{
public:
    text_alignment alignment                   = text_alignment::left;
    text_vertical_alignment vertical_alignment = text_vertical_alignment::top;
    text_word_wrapping word_wrapping           = text_word_wrapping::wrap;
    std::u8string trimming_string              = u8"\u2026";
    char32_t trimming_delimiter                = U'\0';
    std::uint32_t trimming_delimiter_count     = 0;
    text_trimming_granularity trimming_granularity =
        text_trimming_granularity::none;
    thickness padding;
    bool underline = false;
};

std::strong_ordering
strong_order(text_layout_options const&, text_layout_options const&) noexcept;

}

#endif