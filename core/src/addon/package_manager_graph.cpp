#include "addon/package_manager.hpp"
#include "addon/package.hpp"
#include "addon/errors/package_error.hpp"
#include "core.hpp"

#include <gsl/gsl>

std::vector<std::shared_ptr<windower::package const>>
windower::package_manager::load_order() const
{
    std::lock_guard<std::mutex> lock{ m_mutex };
    std::vector<std::u8string> names;
    for (auto const& p : m_installed_packages)
    {
        names.push_back(p.first);
    }
    return load_order_impl(names);
}

std::vector<std::shared_ptr<windower::package const>>
windower::package_manager::load_order(std::vector<std::u8string> const& names) const
{
    std::lock_guard<std::mutex> lock{ m_mutex };
    return load_order_impl(names);
}

std::vector<std::shared_ptr<windower::package const>>
windower::package_manager::unload_order() const
{
    std::lock_guard<std::mutex> lock{ m_mutex };
    std::vector<std::u8string> names;
    for (auto const& p : m_installed_packages)
    {
        names.push_back(p.first);
    }
    return unload_order_impl(names);
}

std::vector<std::shared_ptr<windower::package const>>
windower::package_manager::unload_order(std::vector<std::u8string> const& names) const
{
    std::lock_guard<std::mutex> lock{ m_mutex };
    return unload_order_impl(names);
}

std::vector<std::shared_ptr<windower::package const>>
windower::package_manager::load_order_impl(
    std::vector<std::u8string> const& names) const
{
    std::vector<std::shared_ptr<package const>> results;

    if (!names.empty())
    {
        clear();
        for (auto const& name : names)
        {
            auto const it = m_installed_packages.find(name);
            if (it == m_installed_packages.end())
            {
                core::error(
                    u8"package manager",
                    u8"Cannot load missing package: " + name);
                continue;
            }
            topological_sort(results, name);
        }
    }

    return results;
}

std::vector<std::shared_ptr<windower::package const>>
windower::package_manager::unload_order_impl(
    std::vector<std::u8string> const& names) const
{
    std::vector<std::shared_ptr<package const>> results;

    if (!names.empty())
    {
        clear();
        for (auto const& name : names)
        {
            reverse_topological_sort(results, name);
        }
    }

    return results;
}

void windower::package_manager::clear(vertex_color color) const noexcept
{
    for (auto& p : m_installed_packages)
    {
        p.second.color = color;
    }
}

void windower::package_manager::topological_sort(
    std::vector<std::shared_ptr<package const>>& results,
    std::u8string const& name, bool required) const
{
    auto const it = m_installed_packages.find(name);
    if (it == m_installed_packages.end())
    {
        if (required) throw package_error{ u8"PKG:P1", name };
    }
    else
    {
        auto const color = it->second.color;
        if (color == vertex_color::gray && required)
        {
            core::error(
                u8"package manager",
                u8"Dependency cycle detected in package: " + name);
            return;
        }
        else if (color == vertex_color::white)
        {
            it->second.color = vertex_color::gray;
            for (auto const& d : it->second.value->dependencies())
            {
                topological_sort(results, d.name(), d.required());
            }
            it->second.color = vertex_color::black;
            results.push_back(it->second.value);
        }
    }
}

void windower::package_manager::reverse_topological_sort(
    std::vector<std::shared_ptr<package const>>& results,
    std::u8string const& name) const
{
    auto const package_it = m_installed_packages.find(name);
    if (package_it != m_installed_packages.end())
    {
        for (auto const& vertex : m_installed_packages)
        {
            auto const color = vertex.second.color;
            if (color == vertex_color::white)
            {
                auto& dependencies = vertex.second.value->dependencies();
                auto it = std::find_if(
                    dependencies.begin(), dependencies.end(),
                    [&](auto const& d) noexcept { return d.name() == name; });
                if (it != dependencies.end())
                {
                    vertex.second.color = vertex_color::gray;
                    reverse_topological_sort(
                        results, vertex.second.value->name());
                    vertex.second.color = vertex_color::black;
                }
            }
        }
        results.push_back(package_it->second.value);
    }
}

void windower::package_manager::throw_cycle_error(
    gsl::not_null<package const*> start) const
{
    std::vector<std::u8string> packages;
    auto p = start;
    do
    {
        packages.push_back(p->name());
        for (auto const& d : p->dependencies())
        {
            auto const it = m_installed_packages.find(d.name());
            if (it != m_installed_packages.end() &&
                it->second.color == vertex_color::gray)
            {
                p = it->second.value.get();
                break;
            }
        }
    } while (p != start);
    throw package_error{ u8"PKG:P3", std::move(packages) };
}
