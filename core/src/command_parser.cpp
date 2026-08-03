#include "command_parser.hpp"

#include "errors/syntax_error.hpp"
#include "unicode.hpp"
#include "utilities/debug_helpers.hpp"

#include <array>
#include <gsl/gsl>

namespace windower::command_parser
{

    std::pair<std::optional<u8range>, u8range> parse_command(std::u8string_view command_string)
    {
        constexpr std::array<std::array<std::pair<std::int8_t, std::int8_t>, 5>, 5>
            state_table{ {
                {{{1, 2}, {0, 4}, {0, 4}, {0, 4}, {-1, 4}}},
                {{{0, 7}, {2, 1}, {3, 0}, {0, 7}, {-1, 5}}},
                {{{0, 8}, {0, 6}, {4, 0}, {0, 8}, {-1, 6}}},
                {{{0, 7}, {2, 1}, {3, 0}, {0, 7}, {-1, 3}}},
                {{{0, 8}, {0, 8}, {4, 0}, {0, 8}, {-1, 3}}},
            } };

        constexpr auto next = [](std::u8string_view string,
            std::size_t& offset) -> std::int8_t {
                if (offset == string.size())
                {
                    return 4;
                }
                switch (auto const c = windower::next_code_point(string, offset))
                {
                case U'/': return 0;
                case U':': return 1;
                case U'a': case U'b': case U'c': case U'd': case U'e':
                case U'f': case U'g': case U'h': case U'i': case U'j':
                case U'k': case U'l': case U'm': case U'n': case U'o':
                case U'p': case U'q': case U'r': case U's': case U't':
                case U'u': case U'v': case U'w': case U'x': case U'y': case U'z':
                case U'A': case U'B': case U'C': case U'D': case U'E':
                case U'F': case U'G': case U'H': case U'I': case U'J':
                case U'K': case U'L': case U'M': case U'N': case U'O':
                case U'P': case U'Q': case U'R': case U'S': case U'T':
                case U'U': case U'V': case U'W': case U'X': case U'Y': case U'Z':
                case U'0': case U'1': case U'2': case U'3': case U'4':
                case U'5': case U'6': case U'7': case U'8': case U'9':
                case U'?': case U'_': case U'-': return 2;
                default: return windower::is_whitespace(c) ? 4 : 3;
                }
            };

        auto state = 0;
        auto it = std::size_t{};
        auto mark = it;

        std::pair<std::optional<u8range>, u8range> result;

        while (state >= 0)
        {
            auto next_it = it;
            auto const char_class = next(command_string, next_it);
            auto const [next_state, action] =
                gsl::at(gsl::at(state_table, state), char_class);
            state = next_state;
            switch (action)
            {
            case 0: break;
            case 1: result.first = { mark, it }; [[fallthrough]];
            case 2: mark = next_it; break;
            case 3: result.second = { mark, it }; break;
            case 4: throw syntax_error{ u8"CMD:P1", command_string, it };
            case 5: throw syntax_error{ u8"CMD:P2", command_string, it };
            case 6: throw syntax_error{ u8"CMD:P3", command_string, it };
            case 7: throw syntax_error{ u8"CMD:P4", command_string, it, mark, it };
            case 8: throw syntax_error{ u8"CMD:P5", command_string, it, mark, it };
            default: fail_fast();
            }
            it = next_it;
        }

        return result;
    }

    std::u8string unescape(std::u8string_view string)
    {
        constexpr std::array<std::array<std::pair<std::int8_t, std::int8_t>, 7>, 13>
            state_table{ {
                {{{1, 1}, {0, 0}, { 0, 0}, {0, 0}, {0, 0}, {0, 0}, {-1, 1}}},
                {{{0, 0}, {2, 2}, { 0, 0}, {0, 0}, {0, 0}, {0, 0}, {-1, 1}}},
                {{{1, 1}, {0, 0}, { 3, 3}, {4, 0}, {0, 0}, {0, 0}, {-1, 1}}},
                {{{1, 1}, {0, 0}, { 5, 3}, {0, 0}, {0, 0}, {0, 0}, {-1, 1}}},
                {{{1, 1}, {0, 0}, { 6, 3}, {0, 0}, {0, 0}, {0, 0}, {-1, 1}}},
                {{{1, 1}, {0, 0}, { 7, 3}, {0, 0}, {0, 0}, {0, 0}, {-1, 1}}},
                {{{1, 1}, {0, 0}, { 8, 3}, {0, 0}, {0, 5}, {0, 0}, {-1, 1}}},
                {{{1, 1}, {0, 0}, { 0, 4}, {0, 0}, {0, 5}, {0, 0}, {-1, 1}}},
                {{{1, 1}, {0, 0}, { 9, 3}, {0, 0}, {0, 5}, {0, 0}, {-1, 1}}},
                {{{1, 1}, {0, 0}, {10, 3}, {0, 0}, {0, 5}, {0, 0}, {-1, 1}}},
                {{{1, 1}, {0, 0}, {11, 3}, {0, 0}, {0, 5}, {0, 0}, {-1, 1}}},
                {{{1, 1}, {0, 0}, {12, 3}, {0, 0}, {0, 5}, {0, 0}, {-1, 1}}},
                {{{1, 1}, {0, 0}, { 0, 0}, {0, 0}, {0, 5}, {0, 0}, {-1, 1}}},
            } };

        constexpr auto next = [](std::u8string_view::iterator it,
            std::u8string_view::iterator end)
            -> std::pair<std::int8_t, std::u8string_view::iterator> {
            if (it == end)
            {
                return { 6, end };
            }
            switch (*it++)
            {
            case u8'\\': return { 0, it };
            case u8'u': return { 1, it };
            case u8'0': case u8'1': case u8'2': case u8'3': case u8'4':
            case u8'5': case u8'6': case u8'7': case u8'8': case u8'9':
            case u8'A': case u8'B': case u8'C':
            case u8'D': case u8'E': case u8'F':
            case u8'a': case u8'b': case u8'c':
            case u8'd': case u8'e': case u8'f': return { 2, it };
            case u8'{': return { 3, it };
            case u8'}': return { 4, it };
            default: return { 5, it };
            }
            };

        std::u8string result;

        auto state = 0;
        auto it = string.begin();
        auto mark = it;
        char32_t value = 0;

        while (state >= 0)
        {
            auto const [char_class, next_it] = next(it, string.end());
            auto const [next_state, action] =
                gsl::at(gsl::at(state_table, state), char_class);
            state = next_state;
            switch (action)
            {
            case 0: break;
            case 1:
                result.append(mark, it);
                mark = next_it;
                break;
            case 2: value = 0; break;
            case 3: value = value << 4 | ((*it | 0x1B0) * 0x0E422D48U) >> 28; break;
            case 4:
                value = value << 4 | ((*it | 0x1B0) * 0x0E422D48U) >> 28;
                [[fallthrough]];
            case 5:
                if (value <= U'\U0010FFFF')
                {
                    windower::append(result, value);
                    mark = next_it;
                }
                break;
            default: fail_fast();
            }
            it = next_it;
        }

        return result;
    }

    void parse_arguments(
        std::u8string_view argument_string, std::size_t count,
        std::vector<std::u8string>& output)
    {
        constexpr std::array<std::array<std::pair<std::int8_t, std::int8_t>, 7>, 6>
            state_table{ {
                {{{3, 0}, {2, 1}, {1, 1}, {0, 3}, {0, 1}, {0, 0}, {-1, 1}}},
                {{{4, 0}, {1, 0}, {0, 2}, {1, 0}, {1, 0}, {1, 0}, {-1, 2}}},
                {{{5, 0}, {0, 2}, {2, 0}, {2, 0}, {2, 0}, {2, 0}, {-1, 2}}},
                {{{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {-1, 1}}},
                {{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {-1, 1}}},
                {{{2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0}, {-1, 1}}},
            } };

        constexpr auto next = [](std::u8string_view string,
            std::size_t& offset) -> std::int8_t {
                if (offset == string.size())
                {
                    return 6;
                }
                switch (auto const c = windower::next_code_point(string, offset))
                {
                case u8'\\': return 0;
                case u8'\'': return 1;
                case u8'"': return 2;
                default:
                    if ((c >= U'\U000F0000' && c <= U'\U000FFFFD') ||
                        (c >= U'\U00100000' && c <= U'\U0010FFFD'))
                    {
                        return 3;
                    }
                    if (windower::is_whitespace(c))
                    {
                        return 4;
                    }
                    return 5;
                }
            };

        constexpr auto append = [](std::vector<std::u8string>& output,
            std::u8string_view string,
            std::size_t begin_offset, std::size_t end_offset,
            std::size_t& count, bool allow_empty = false) {
                if (begin_offset == end_offset)
                {
                    if (allow_empty)
                    {
                        output.emplace_back();
                    }
                }
                else
                {
                    auto const size = end_offset - begin_offset;
                    output.push_back(unescape(string.substr(begin_offset, size)));
                }
                return count != 0;
            };

        auto state = 0;
        auto it = std::size_t{};
        auto mark = it;

        while (count != 0 && state >= 0)
        {
            auto next_it = it;
            auto const char_class = next(argument_string, next_it);
            auto const [next_state, action] =
                gsl::at(gsl::at(state_table, state), char_class);
            state = next_state;
            switch (action)
            {
            case 0: break;
            case 1:
                mark =
                    append(output, argument_string, mark, it, count) ? next_it : it;
                break;
            case 2:
                append(output, argument_string, mark, it, count, true);
                mark = next_it;
                break;
            case 3:
                if (append(output, argument_string, mark, it, count))
                {
                    append(output, argument_string, it, next_it, count);
                    mark = next_it;
                    break;
                }
                mark = it;
                break;
            default: fail_fast();
            }
            it = next_it;
        }

        if (mark != argument_string.size())
        {
            output.emplace_back(argument_string.substr(mark));
        }
    }

    std::u8string_view substring(
        std::u8string_view string,
        u8range range) noexcept
    {
        return string.substr(range.first, range.second - range.first);
    }

    std::optional<std::u8string_view> substring(
        std::u8string_view string,
        std::optional<u8range> range) noexcept
    {
        if (!range)
        {
            return std::nullopt;
        }
        return substring(string, *range);
    }

}
