#include "errors/xml_error.hpp"

#include <memory>
#include <string>
#include <string_view>

windower::xml_error::xml_error(std::u8string_view description) :
    xml_error{description, {}, {}, 0, 0}
{}

windower::xml_error::xml_error(
    std::u8string_view description, std::filesystem::path path) :
    xml_error{description, std::move(path), {}, 0, 0}
{}

windower::xml_error::xml_error(
    std::u8string_view description, std::u8string_view line_source,
    std::size_t line, std::size_t column) :
    xml_error{description, {}, line_source, line, column}
{}

windower::xml_error::xml_error(
    std::u8string_view description, std::filesystem::path path,
    std::u8string_view line_source, std::size_t line, std::size_t column) :
    syntax_error{u8"XML", line_source, column},
    m_path{std::make_shared<std::filesystem::path>(std::move(path))},
    m_description{std::make_shared<std::u8string>(description)}, m_line{line}
{}

std::filesystem::path const& windower::xml_error::path() const noexcept
{
    return *m_path;
}

std::u8string const& windower::xml_error::description() const noexcept
{
    return *m_description;
}

std::size_t windower::xml_error::line() const noexcept { return m_line; }