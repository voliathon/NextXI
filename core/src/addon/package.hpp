#ifndef WINDOWER_ADDON_PACKAGE_HPP
#define WINDOWER_ADDON_PACKAGE_HPP

#include <compare>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace windower
{
    struct package_version
    {
        std::uint32_t major;
        std::uint32_t minor;
        std::uint32_t revision;
        std::uint32_t build;
        std::u8string tag;

        explicit package_version(
            std::uint32_t = 0, std::uint32_t = 0, std::uint32_t = 0,
            std::uint32_t = 0, std::u8string_view = u8"") noexcept;
        explicit package_version(std::u8string_view);
        explicit package_version(std::string_view);

        // CAVEMAN FIX: Only default the spaceship operator! 
        // C++20 automatically generates <, <=, >, and >= from this.
        std::weak_ordering operator<=>(package_version const&) const noexcept = default;
    };

    std::u8string to_u8string(package_version const&);

    enum class package_type
    {
        library,
        addon,
        service,
    };

    class package_dependency
    {
    public:
        explicit package_dependency(std::u8string_view, bool = true);
        std::u8string const& name() const noexcept;
        bool required() const noexcept;
    private:
        std::u8string m_name;
        bool m_required;
    };

    class package
    {
    public:
        explicit package(std::filesystem::path root_path, bool can_update);

        std::u8string const& name() const noexcept;
        package_version const& version() const noexcept;
        package_type type() const noexcept;
        std::vector<package_dependency> const& dependencies() const noexcept;
        std::filesystem::path const& path() const noexcept;

        std::ifstream resolve(std::filesystem::path const&) const;
        std::filesystem::path absolute_path(std::filesystem::path const& relative_path) const;

        bool can_update() const noexcept;

    private:
        std::filesystem::path m_root_path;
        std::u8string m_name;
        package_version m_version;
        package_type m_type;
        std::vector<package_dependency> m_dependencies;
        bool m_can_update;
    };
}

#endif
