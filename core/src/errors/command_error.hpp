#ifndef WINDOWER_ERRORS_COMMAND_ERROR_HPP
#define WINDOWER_ERRORS_COMMAND_ERROR_HPP

#include "errors/windower_error.hpp"

#include <memory>
#include <string>
#include <string_view>

namespace windower
{

class command_error : public windower_error
{
public:
    command_error(std::u8string_view error_code, std::u8string_view command);

    std::u8string const& command() const noexcept;

private:
    std::shared_ptr<std::u8string> m_command;
};

}

#endif