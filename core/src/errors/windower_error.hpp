#ifndef WINDOWER_ERRORS_WINDOWER_ERROR_HPP
#define WINDOWER_ERRORS_WINDOWER_ERROR_HPP

#include <exception>
#include <memory>
#include <string>
#include <string_view>

namespace windower
{

class windower_error : public std::exception, public std::nested_exception
{
public:
    windower_error(std::u8string_view error_code);

    char const* what() const noexcept override;

    std::u8string const& error_code() const noexcept;
    std::u8string_view message() const;

private:
    std::shared_ptr<std::u8string const> m_error_code;
};

}

#endif