#ifndef WINDOWER_UTILITIES_NULL_OUTPUT_HPP
#define WINDOWER_UTILITIES_NULL_OUTPUT_HPP

#include <cstddef>
#include <iterator>
#include <tuple>

class null_output_iterator
{
public:
    using iterator_category = std::output_iterator_tag;
    using value_type        = decltype(std::ignore);
    using difference_type   = std::ptrdiff_t;
    using pointer           = value_type*;
    using reference         = value_type&;

    reference operator*() noexcept { return std::ignore; }
    null_output_iterator& operator++() noexcept { return *this; }
    null_output_iterator& operator++(int) noexcept { return *this; }
};

constexpr auto null_output = null_output_iterator{};

#endif