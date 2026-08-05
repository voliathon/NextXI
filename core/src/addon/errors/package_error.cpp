#include "addon/errors/package_error.hpp"

windower::package_error::package_error(std::u8string_view error_code) :
    package_error{error_code, std::vector<std::u8string>{}}
{}

windower::package_error::package_error(
    std::u8string_view error_code, std::u8string_view package) :
    package_error{
        error_code, std::vector<std::u8string>{std::u8string{package}}}
{}

windower::package_error::package_error(
    std::u8string_view error_code, std::vector<std::u8string> packages) :
    windower_error{error_code},
    m_packages{
        std::make_shared<std::vector<std::u8string>>(std::move(packages))}
{}

std::vector<std::u8string> const&
windower::package_error::packages() const noexcept
{
    return *m_packages;
}