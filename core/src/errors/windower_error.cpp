#include "errors/windower_error.hpp"

#include "errors/detail/error_messages.hpp"

#include <gsl/gsl>

#include <array>

namespace
{

template<typename I>
constexpr void sort(I begin, I end) noexcept
{
    while (begin != end)
    {
        auto const front = begin++;
        auto select      = front;
        auto select_size =
            std::char_traits<char8_t>::length(std::get<0>(*select));
        for (auto it = begin; it != end; ++it)
        {
            auto const it_size =
                std::char_traits<char8_t>::length(std::get<0>(*it));
            auto const min_size = it_size < select_size ? it_size : select_size;
            if (std::char_traits<char8_t>::compare(
                    std::get<0>(*it), std::get<0>(*select), min_size) < 0)
            {
                select      = it;
                select_size = it_size;
            }
        }
        std::swap(*front, *select);
    }
}

}

windower::windower_error::windower_error(std::u8string_view message) :
    m_error_code{std::make_shared<std::u8string>(message)}
{}

char const* windower::windower_error::what() const noexcept
{
    GSL_SUPPRESS("type.1")
    {
        return reinterpret_cast<char const*>(m_error_code->c_str());
    }
}

std::u8string const& windower::windower_error::error_code() const noexcept
{
    return *m_error_code;
}

std::u8string_view windower::windower_error::message() const
{
    static constexpr auto sorted = []() {
        auto result = detail::error_messages;
        ::sort(result.begin(), result.end());
        return result;
    }();
    if (auto it = std::lower_bound(
            sorted.begin(), sorted.end(), *m_error_code,
            [](auto const& a, auto b) { return std::get<0>(a) < b; });
        it != sorted.end() && std::get<0>(*it) == *m_error_code)
    {
        return std::get<1>(*it);
    }
    return u8"<UNKNOWN ERROR>";
}