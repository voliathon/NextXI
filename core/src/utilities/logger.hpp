#ifndef WINDOWER_UTILITIES_LOGGER_HPP
#define WINDOWER_UTILITIES_LOGGER_HPP

#include <exception>
#include <string>
#include <string_view>

namespace windower::logger
{
    std::u8string process_output(std::u8string_view component, std::u8string_view text);

    void queue_log(std::u8string text, bool is_error);

    std::u8string get_error_message(std::exception const& exception);
}

#endif
