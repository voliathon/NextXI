#ifndef WINDOWER_ADDON_MODULES_CHAT_HPP
#define WINDOWER_ADDON_MODULES_CHAT_HPP

#include "addon/lua.hpp"
#include "addon/modules/event.hpp"

#include <string>
#include <string_view>

namespace windower
{

namespace detail
{

struct text_added_result_data
{
    text_added_result_data(
        std::u8string text, std::uint8_t type, bool indented) :
        text{text},
        type{type}, indented{indented}
    {}

    std::u8string text;
    std::uint8_t type;
    bool indented;
};

}

class text_added_result : public basic_result<detail::text_added_result_data>
{
public:
    using basic_result::basic_result;

    std::u8string_view text() const noexcept;
    std::uint8_t type() const noexcept;
    bool indented() const noexcept;
};

text_added_result
trigger_text_added(std::u8string_view text, std::uint8_t type, bool indented);

int load_chat_module(lua::state);

}

#endif