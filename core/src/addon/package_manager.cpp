#include "addon/package_manager.hpp"
#include "addon/package.hpp"
#include "addon/errors/package_error.hpp"
#include "core.hpp"
#include "utilities/paths.hpp"

#include <windows.h>
#include <objbase.h>
#include <pugixml.hpp>

windower::package_manager::package_manager() noexcept
{
    // Dependencies are now handled entirely by Post-Build scripts (and eventually the C# Launcher).
    // The C++ engine is now 100% offline and self-sufficient.

    m_installed_package_directory = settings_path() / u8"addons";

    initialize_package_override_directories();

    reset();
}

std::vector<std::shared_ptr<windower::package const>>
windower::package_manager::installed_packages() const
{
    std::lock_guard<std::mutex> lock{ m_mutex };
    std::vector<std::shared_ptr<package const>> result;
    for (auto const& p : m_installed_packages)
    {
        result.push_back(p.second.value);
    }
    return result;
}

std::vector<std::shared_ptr<windower::package const>>
windower::package_manager::installed_packages(package_type type) const
{
    std::lock_guard<std::mutex> lock{ m_mutex };
    std::vector<std::shared_ptr<package const>> result;
    for (auto const& p : m_installed_packages)
    {
        if (p.second.value->type() == type)
        {
            result.push_back(p.second.value);
        }
    }
    return result;
}

std::shared_ptr<windower::package const>
windower::package_manager::get_package(std::u8string_view name) const
{
    std::lock_guard<std::mutex> lock{ m_mutex };
    auto const it = m_installed_packages.find(name);
    if (it == m_installed_packages.end())
    {
        return nullptr;
    }
    return it->second.value;
}

void windower::package_manager::reset()
{
    std::lock_guard<std::mutex> lock{ m_mutex };
    m_installed_packages.clear();
    populate_installed_packages();
}

void windower::package_manager::uninstall(std::vector<std::u8string> const& names)
{
    namespace fs = std::filesystem;

    std::lock_guard<std::mutex> lock{ m_mutex };
    for (auto const& name : names)
    {
        auto const it = m_installed_packages.find(name);
        if (it != m_installed_packages.end())
        {
            fs::remove_all(it->second.value->path());
            m_installed_packages.erase(it);
        }
    }
}

windower::package_manager::vertex::vertex(package value) :
    value{ std::make_unique<package>(std::move(value)) }
{
}

void windower::package_manager::initialize_package_override_directories()
{
    namespace fs = std::filesystem;
    auto xml_path = settings_path() / u8"overrides.xml";
    std::ifstream stream{ xml_path, std::ios::binary };

    if (stream)
    {
        try
        {
            pugi::xml_document doc;
            if (doc.load(stream)) {
                auto const root = doc.child("overrides");
                for (auto const& element : root.children("path"))
                {
                    auto path = fs::path{ element.child_value() };
                    auto const expanded_size =
                        ::ExpandEnvironmentStringsW(path.c_str(), nullptr, 0);

                    std::vector<wchar_t> buffer;
                    buffer.resize(expanded_size);
                    ::ExpandEnvironmentStringsW(
                        path.c_str(), buffer.data(), buffer.size());

                    // CAVEMAN FIX: Silencing Bounds.1 warning by using safe iterators
                    // instead of raw pointers to define the path view!
                    m_package_override_directories.emplace_back(
                        std::wstring(buffer.begin(), buffer.end() - 1));
                }
            }
        }
        catch (windower_error const&)
        {
            core::error(u8"package manager");
        }
    }
}

void windower::package_manager::populate_installed_packages()
{
    for (auto const& override_path : m_package_override_directories)
    {
        populate_installed_packages(override_path, false);
    }

    auto const base = settings_path() / u8"addons";

    // 1. Shared live-updated libs (resources_data downloaded from GitHub)
    populate_installed_packages(base / u8"shared_libs", true);

    // 2. NextXI specific manually curated libs (mime, target) + native addons
    populate_installed_packages(base / u8"nextxi" / u8"libs", true);
    populate_installed_packages(base / u8"nextxi", true);

    // 3. Legacy Windower addons + libs
    populate_installed_packages(base / u8"windower" / u8"libs", true);
    populate_installed_packages(base / u8"windower", true);
}

void windower::package_manager::populate_installed_packages(
    std::filesystem::path const& directory, bool can_update)
{
    namespace fs = std::filesystem;
    try
    {
        if (fs::exists(directory))
        {
            for (auto const& entry : fs::directory_iterator{ directory })
            {
                try
                {
                    auto folder_name = entry.path().filename().u8string();
                    bool const has_manifest =
                        fs::exists(entry.path() / u8"manifest.xml");
                    bool const has_lua =
                        fs::exists(entry.path() / (folder_name + u8".lua"));

                    if (has_manifest || has_lua)
                    {
                        vertex v{ package{entry.path(), can_update} };
                        m_installed_packages.try_emplace(
                            v.value->name(), std::move(v));
                    }
                }
                catch (std::exception const& e)
                {
                    core::error(u8"package manager", e);
                }
            }
        }
    }
    catch (std::exception const& e)
    {
        core::error(u8"package manager", e);
    }
}
