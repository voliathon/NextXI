#ifndef WINDOWER_ERRORS_SYNTAX_ERROR_HPP
#define WINDOWER_ERRORS_SYNTAX_ERROR_HPP

#include "errors/windower_error.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace windower
{

class syntax_error : public windower_error
{
public:
    syntax_error(std::u8string_view error_code);

    syntax_error(
        std::u8string_view error_code, std::u8string_view source,
        std::size_t mark_index);

    syntax_error(
        std::u8string_view error_code, std::u8string_view source,
        std::size_t mark_index, std::size_t begin_index, std::size_t end_index);

    syntax_error(
        std::u8string_view error_code, std::u8string_view source,
        std::size_t mark_index, std::size_t begin_index, std::size_t end_index,
        std::vector<std::u8string> options);

    std::u8string const& source() const noexcept;
    std::size_t mark_index() const noexcept;
    std::size_t begin_index() const noexcept;
    std::size_t end_index() const noexcept;
    std::vector<std::u8string> const& options() const noexcept;

private:
    std::shared_ptr<std::u8string const> m_source;
    std::shared_ptr<std::vector<std::u8string> const> m_options;
    std::size_t m_mark_index;
    std::size_t m_begin_index;
    std::size_t m_end_index;
};

}

#endif