#ifndef WINDOWER_ADDON_PACKAGE_MANAGER_HPP
#define WINDOWER_ADDON_PACKAGE_MANAGER_HPP

#include "addon/package.hpp"
#include "errors/windower_error.hpp"

#include <gsl/gsl>

#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

namespace windower
{
    class package_manager
    {
    public:
        package_manager() noexcept;

        std::vector<std::shared_ptr<windower::package const>> installed_packages() const;
        std::vector<std::shared_ptr<windower::package const>> installed_packages(package_type) const;

        std::vector<std::shared_ptr<windower::package const>> load_order() const;
        std::vector<std::shared_ptr<windower::package const>> load_order(std::vector<std::u8string> const&) const;

        std::vector<std::shared_ptr<windower::package const>> unload_order() const;
        std::vector<std::shared_ptr<windower::package const>> unload_order(std::vector<std::u8string> const&) const;

        std::shared_ptr<windower::package const> get_package(std::u8string_view) const;

        void reset();

        std::vector<std::u8string> sources();
        std::future<void> add_source(std::u8string_view);
        void remove_source(std::u8string_view);

        std::future<std::vector<std::u8string>> install(std::vector<std::u8string> const&);
        std::future<std::vector<std::u8string>> update(std::vector<std::u8string> const&);
        std::future<std::vector<std::u8string>> update_all(bool = false);

        void uninstall(std::vector<std::u8string> const&);

    private:
        enum class vertex_color
        {
            white,
            gray,
            black
        };

        struct vertex
        {
            std::shared_ptr<package> value;
            mutable vertex_color color = vertex_color::white;

            explicit vertex(package);
        };

        struct source
        {
            std::u8string url;
            std::u8string guid;
            bool built_in = false;
        };

        struct descriptor
        {
            std::u8string name;
            package_version version;
            std::vector<std::u8string> dependencies;
            std::u8string root_url;
            std::vector<std::filesystem::path> files;
        };

        mutable std::mutex m_mutex;
        std::vector<source> m_package_sources;
        std::vector<descriptor> m_available_packages;
        std::filesystem::path m_installed_package_directory;
        std::vector<std::filesystem::path> m_package_override_directories;
        std::map<std::u8string, vertex, std::less<>> m_installed_packages;

        void initialize_source_list();
        void initialize_package_override_directories();

        void save_source_list();

        std::future<void> update_sources(bool);
        std::future<void> update_source(source, bool);
        void reload_sources();

        std::vector<descriptor> out_of_date(std::vector<std::u8string> const&) const;
        std::future<std::vector<std::u8string>> install_or_update(std::vector<std::u8string>, bool);

        void populate_installed_packages();
        void populate_installed_packages(std::filesystem::path const&, bool);
        void load_source(std::filesystem::path const&, std::u8string const&);

        std::vector<std::shared_ptr<package const>> load_order_impl(std::vector<std::u8string> const&) const;
        std::vector<std::shared_ptr<package const>> unload_order_impl(std::vector<std::u8string> const&) const;

        void clear(vertex_color = vertex_color::white) const noexcept;
        void topological_sort(std::vector<std::shared_ptr<package const>>&, std::u8string const&, bool = true) const;
        void reverse_topological_sort(std::vector<std::shared_ptr<package const>>&, std::u8string const&) const;
        [[noreturn]] void throw_cycle_error(gsl::not_null<package const*>) const;
    };
}

#endif
