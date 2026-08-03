#include "ui/text_layout_options.hpp"

#include <gsl/gsl>

namespace windower::ui
{

std::strong_ordering strong_order(
    text_layout_options const& lhs, text_layout_options const& rhs) noexcept
{
    if (auto const result = lhs.alignment <=> rhs.alignment; result != 0)
    {
        return result;
    }

    if (auto const result = lhs.vertical_alignment <=> rhs.vertical_alignment;
        result != std::strong_ordering::equal)
    {
        return result;
    }

    if (auto const result = lhs.word_wrapping <=> rhs.word_wrapping;
        result != std::strong_ordering::equal)
    {
        return result;
    }

    if (auto const result = lhs.trimming_string <=> rhs.trimming_string;
        result != std::strong_ordering::equal)
    {
        return result;
    }

    if (auto const result = lhs.trimming_delimiter <=> rhs.trimming_delimiter;
        result != std::strong_ordering::equal)
    {
        return result;
    }

    if (auto const result =
            lhs.trimming_delimiter_count <=> rhs.trimming_delimiter_count;
        result != std::strong_ordering::equal)
    {
        return result;
    }

    if (auto const result =
            lhs.trimming_granularity <=> rhs.trimming_granularity;
        result != std::strong_ordering::equal)
    {
        return result;
    }

    if (auto const result =
            std::strong_order(lhs.padding.left, rhs.padding.left);
        result != std::strong_ordering::equal)
    {
        return result;
    }

    if (auto const result = std::strong_order(lhs.padding.top, rhs.padding.top);
        result != std::strong_ordering::equal)
    {
        return result;
    }

    if (auto const result =
            std::strong_order(lhs.padding.right, rhs.padding.right);
        result != std::strong_ordering::equal)
    {
        return result;
    }

    if (auto const result =
            std::strong_order(lhs.padding.bottom, rhs.padding.bottom);
        result != std::strong_ordering::equal)
    {
        return result;
    }

    return lhs.underline <=> rhs.underline;
}

}