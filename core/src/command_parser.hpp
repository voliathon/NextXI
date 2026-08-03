#ifndef WINDOWER_COMMAND_PARSER_HPP
#define WINDOWER_COMMAND_PARSER_HPP

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace windower::command_parser
{
    using u8range = std::pair<std::size_t, std::size_t>;

    std::pair<std::optional<u8range>, u8range> parse_command(std::u8string_view command_string);

    std::u8string unescape(std::u8string_view string);

    void parse_arguments(
        std::u8string_view argument_string, std::size_t count,
        std::vector<std::u8string>& output);

    std::u8string_view substring(
        std::u8string_view string, u8range range) noexcept;

    std::optional<std::u8string_view> substring(
        std::u8string_view string, std::optional<u8range> range) noexcept;
}

#endif
