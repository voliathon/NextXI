#include "addon/package.hpp"
#include "addon/errors/package_error.hpp"
#include "utilities/paths.hpp"
#include "utility.hpp"

#include <pugixml.hpp>
#include <charconv>

windower::package_version::package_version(
    std::uint32_t major, std::uint32_t minor, std::uint32_t revision,
    std::uint32_t build, std::u8string_view tag) noexcept :
    major{ major }, minor{ minor }, revision{ revision }, build{ build }, tag{ tag }
{
}

windower::package_version::package_version(std::u8string_view value) :
    package_version(to_string_view(value))
{
}

windower::package_version::package_version(std::string_view value) :
    minor{}, revision{}, build{}
{
    value.remove_prefix(parse(value, major));
    if (!value.empty() && value.front() != u8'.')
    {
        value.remove_prefix(parse(value, minor));
        if (!value.empty() && value.front() != u8'.')
        {
            value.remove_prefix(parse(value, revision));
            if (!value.empty() && value.front() != u8'.')
            {
                value.remove_prefix(parse(value, build));
            }
        }
    }

    while (!value.empty() && value.front() == u8' ')
    {
        value.remove_prefix(1);
    }

    tag = to_u8string(value);
}

std::u8string windower::to_u8string(package_version const& value)
{
    std::u8string result;
    result.append(to_u8string(value.major)).append(1, u8'.');
    result.append(to_u8string(value.minor)).append(1, u8'.');
    result.append(to_u8string(value.revision)).append(1, u8'.');
    result.append(to_u8string(value.build));
    if (!value.tag.empty())
    {
        result.append(1, u8' ').append(value.tag);
    }
    return result;
}

windower::package_dependency::package_dependency(
    std::u8string_view name, bool required) : m_name{ name }, m_required{ required }
{
}

std::u8string const& windower::package_dependency::name() const noexcept
{
    return m_name;
}

bool windower::package_dependency::required() const noexcept
{
    return m_required;
}

windower::package::package(std::filesystem::path root_path, bool can_update) :
    m_root_path{ std::move(root_path) }, m_can_update{ can_update }
{
    pugi::xml_document doc;
    if (std::filesystem::exists(m_root_path / u8"manifest.xml"))
    {
        auto stream = resolve(u8"manifest.xml");
        if (!doc.load(stream)) throw package_error{ u8"PKG:F2" };
    }
    else
    {
        auto folder_name = m_root_path.filename().u8string();
        auto root = doc.append_child("package");
        root.append_child("name").text().set(
            reinterpret_cast<char const*>(folder_name.c_str()));
        root.append_child("version").text().set("1.0.0");
        root.append_child("type").text().set("addon");
        root.append_child("description").text().set("Auto-generated manifest");
    }

    auto const package = doc.child("package");
    if (!package)
    {
        throw package_error{ u8"PKG:M1" };
    }

    m_name = to_u8string(package.child_value("name"));
    m_version = package_version{ package.child("version").text().as_string() };

    auto const type =
        std::string_view{ package.child("type").text().as_string("addon") };
    if (type == "addon")
    {
        m_type = package_type::addon;
    }
    else if (type == "service")
    {
        m_type = package_type::service;
    }
    else if (type == "library" || type == "data" || type == "plugin")
    {
        m_type = package_type::library;
    }
    else
    {
        throw package_error{ u8"PKG:M2" };
    }

    auto const dependencies = package.child("dependencies");
    for (auto const& dependency : dependencies.children("dependency"))
    {
        std::string_view const value = dependency.child_value();
        std::u8string name;
        name.assign(value.begin(), value.end());
        m_dependencies.emplace_back(
            name, !dependency.attribute("optional").as_bool());
    }
}

std::u8string const& windower::package::name() const noexcept { return m_name; }

windower::package_version const& windower::package::version() const noexcept
{
    return m_version;
}

windower::package_type windower::package::type() const noexcept
{
    return m_type;
}

std::vector<windower::package_dependency> const&
windower::package::dependencies() const noexcept
{
    return m_dependencies;
}

std::filesystem::path const& windower::package::path() const noexcept
{
    return m_root_path;
}

std::ifstream
windower::package::resolve(std::filesystem::path const& relative_path) const
{
    auto path = absolute_path(relative_path);
    if (auto stream = std::ifstream{ path, std::ios::binary }; stream.is_open())
    {
        return stream;
    }
    throw package_error{ u8"PKG:F2" };
}

std::filesystem::path windower::package::absolute_path(
    std::filesystem::path const& relative_path) const
{
    auto const type = std::filesystem::status(m_root_path).type();
    if (type == std::filesystem::file_type::directory)
    {
        return m_root_path / relative_path;
    }
    throw package_error{ u8"PKG:F1" };
}

bool windower::package::can_update() const noexcept { return m_can_update; }
