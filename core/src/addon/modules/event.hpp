#ifndef WINDOWER_ADDON_MODULES_EVENT_HPP
#define WINDOWER_ADDON_MODULES_EVENT_HPP

#include "addon/lua.hpp"
#include "core.hpp"

#include <variant>

namespace windower
{

class block_t
{
public:
    explicit constexpr block_t() = default;
};

constexpr block_t block = block_t{};

template<typename T>
class basic_result
{
public:
    constexpr basic_result() noexcept : m_data{std::in_place_index<1>} {}
    constexpr basic_result(block_t) noexcept : m_data{std::in_place_index<0>} {}
    template<typename Arg, typename... Args>
    basic_result(Arg&& arg, Args&&... args) noexcept :
        m_data{std::in_place_index<2>, arg, args...}
    {}

    bool blocked() const noexcept { return m_data.index() == 0; }
    bool unchanged() const noexcept { return m_data.index() == 1; }

protected:
    T const& wrapped_value() const noexcept { return *std::get_if<2>(&m_data); }

private:
    std::variant<std::monostate, std::monostate, T> m_data;
};

template<typename F>
void run_on_all_interpreters(F&& function)
{
    auto const& core = core::instance();

    if (auto s = core.script_environment.root_handle().lock())
    {
        function(*s);
    }
    if (core.addon_manager)
    {
        for (auto const& addon : core.addon_manager->loaded())
        {
            if (auto s = addon->root_handle().lock())
            {
                function(*s);
            }
        }
    }
}

int load_event_module(lua::state);

}

#endif