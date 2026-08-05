#ifndef WINDOWER_ERRORS_XML_ERROR_HPP
#define WINDOWER_ERRORS_XML_ERROR_HPP

#include "errors/syntax_error.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace windower
{

class xml_error : public syntax_error
{
public:
    xml_error(std::u8string_view description);
    xml_error(std::u8string_view description, std::filesystem::path path);
    xml_error(
        std::u8string_view description, std::u8string_view line_source,
        std::size_t line, std::size_t column);
    xml_error(
        std::u8string_view description, std::filesystem::path path,
        std::u8string_view line_source, std::size_t line, std::size_t column);

    std::filesystem::path const& path() const noexcept;
    std::u8string const& description() const noexcept;
    std::size_t line() const noexcept;

private:
    std::shared_ptr<std::filesystem::path> m_path;
    std::shared_ptr<std::u8string> m_description;
    std::size_t m_line;
};

}

#endif