#include "errors/command_error.hpp"

#include "errors/windower_error.hpp"

#include <memory>
#include <string>
#include <string_view>

windower::command_error::command_error(
    std::u8string_view error_code, std::u8string_view command) :
    windower_error{error_code},
    m_command{std::make_shared<std::u8string>(command)}
{}

std::u8string const& windower::command_error::command() const noexcept
{
    return *m_command;
}