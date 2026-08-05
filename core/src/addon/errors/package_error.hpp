#ifndef WINDOWER_ADDON_ERRORS_PACKAGE_ERROR_HPP
#define WINDOWER_ADDON_ERRORS_PACKAGE_ERROR_HPP

#include "errors/windower_error.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace windower
{

class package_error : public windower_error
{
public:
    explicit package_error(std::u8string_view error_code);
    explicit package_error(
        std::u8string_view error_code, std::u8string_view package);
    explicit package_error(
        std::u8string_view error_code, std::vector<std::u8string> packages);

    std::vector<std::u8string> const& packages() const noexcept;

private:
    std::shared_ptr<std::vector<std::u8string>> m_packages;
};

}

#endif