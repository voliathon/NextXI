#include "addon/error.hpp"

#include "addon/lua.hpp"
#include "addon/unsafe.hpp"
#include "utility.hpp"

#include <lua.hpp>

#include <memory>
#include <stdexcept>
#include <vector>

windower::lua::error::error(state s) :
    std::runtime_error{windower::to_string(lua::get<std::u8string>(s, -1))},
    m_stack_trace{unsafe::get_stack_trace(unsafe::unwrap(s))}
{}

windower::lua::error::error(std::string const& message, state s) :
    std::runtime_error{message}, m_stack_trace{
                                     unsafe::get_stack_trace(unsafe::unwrap(s))}
{}

windower::lua::error::error(char const* message, state s) :
    std::runtime_error{message}, m_stack_trace{
                                     unsafe::get_stack_trace(unsafe::unwrap(s))}
{}

windower::lua::error::error(std::string const& message) :
    std::runtime_error{message}
{}

windower::lua::error::error(char const* message) : std::runtime_error{message}
{}

bool windower::lua::error::has_stack_trace() const noexcept
{
    return m_stack_trace != nullptr;
}

std::vector<windower::lua::stack_frame> const&
windower::lua::error::stack_trace() const noexcept
{
    return *m_stack_trace;
}