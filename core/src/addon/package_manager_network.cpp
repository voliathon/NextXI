#include "addon/package_manager.hpp"
#include "addon/package.hpp"
#include "addon/errors/package_error.hpp"
#include "core.hpp"
#include "downloader.hpp"
#include "utilities/coroutine.hpp"
#include "utilities/xml.hpp"
#include "utilities/paths.hpp"
#include "utility.hpp"

#include <pugixml.hpp>
#include <gsl/gsl>
#include <chrono>
#include <algorithm>

namespace
{
    bool check(windower::downloader::result const& result)
    {
        using namespace windower;
        if (result) return true;

        if (result.is_http_error())
        {
            std::u8string message;
            message.append(u8"HTTP Error ");
            message.append(to_u8string(result.http_status()));
            message.append(1, u8'\n');
            message.append(result.file().url);
            core::error(u8"package manager", message);
        }
        else
        {
            std::u8string message;
            auto temp = result.error_code().message();
            std::copy(temp.begin(), temp.end(), std::back_inserter(message));
            message.append(1, u8'\n');
            message.append(result.file().path.u8string());
            core::error(u8"package manager", message);
        }
        return false;
    }

    std::optional<std::chrono::system_clock::time_point>
        last_modified(std::filesystem::path const& path, bool force)
    {
        if (!force)
        {
            try
            {
                auto const last_write_time = std::filesystem::last_write_time(path);
                return std::chrono::clock_cast<std::chrono::system_clock>(last_write_time);
            }
            catch (std::filesystem::filesystem_error const&) {}
        }
        return std::nullopt;
    }
}

std::vector<std::u8string> windower::package_manager::sources()
{
    std::vector<std::u8string> results;
    {
        std::lock_guard<std::mutex> lock{ m_mutex };
        for (auto const& src : m_package_sources)
        {
            if (!src.built_in) results.push_back(src.url);
        }
    }
    return results;
}

std::future<void> windower::package_manager::add_source(std::u8string_view url)
{
    {
        std::lock_guard<std::mutex> lock{ m_mutex };
        auto it = std::find_if(
            m_package_sources.begin(), m_package_sources.end(),
            [&](auto const& s) { return s.url == url; });
        if (it != m_package_sources.end()) throw package_error{ u8"PKG:S1" };
    }

    auto const guid = guid::generate();
    auto const guid_string = guid.string();
    source src{ std::u8string{url}, guid_string, false };
    co_await update_source(src, true);
    m_package_sources.push_back(src);
    reload_sources();
    save_source_list();
}

void windower::package_manager::remove_source(std::u8string_view url)
{
    {
        std::lock_guard<std::mutex> lock{ m_mutex };
        auto it = std::remove_if(
            m_package_sources.begin(), m_package_sources.end(),
            [&](auto const& s) { return s.url == url; });
        auto const count = std::distance(it, m_package_sources.end());
        m_package_sources.erase(it, m_package_sources.end());
        if (count == 0) throw package_error{ u8"PKG:S2" };
    }
    save_source_list();
    reload_sources();
}

std::future<std::vector<std::u8string>>
windower::package_manager::install(std::vector<std::u8string> const& names)
{
    std::vector<std::u8string> temp;
    {
        std::lock_guard<std::mutex> lock{ m_mutex };
        std::copy_if(
            names.begin(), names.end(), std::back_inserter(temp),
            [=](auto const& n) {
                auto it = std::find_if(
                    m_installed_packages.begin(), m_installed_packages.end(),
                    [&](auto const& p) noexcept { return p.first == n; });
                return it == m_installed_packages.end();
            });
    }
    co_await update_sources(false);
    reload_sources();
    co_return co_await install_or_update(temp, false);
}

std::future<std::vector<std::u8string>>
windower::package_manager::update(std::vector<std::u8string> const& names)
{
    std::vector<std::u8string> temp;
    {
        std::lock_guard<std::mutex> lock{ m_mutex };
        std::copy_if(
            names.begin(), names.end(), std::back_inserter(temp),
            [=](auto const& n) {
                auto it = std::find_if(
                    m_installed_packages.begin(), m_installed_packages.end(),
                    [&](auto const& p) noexcept { return p.first == n; });
                return it != m_installed_packages.end();
            });
    }
    co_await update_sources(false);
    reload_sources();
    co_return co_await install_or_update(temp, false);
}

std::future<std::vector<std::u8string>>
windower::package_manager::update_all(bool force)
{
    std::vector<std::u8string> names;
    {
        std::lock_guard<std::mutex> lock{ m_mutex };
        for (auto const& p : m_installed_packages)
        {
            names.push_back(p.first);
        }
    }
    co_await update_sources(force);
    reload_sources();
    co_return co_await install_or_update(names, force);
}

void windower::package_manager::initialize_source_list()
{
    auto xml_path = settings_path() / u8"updates" / u8"source_list";
    std::ifstream stream{ xml_path, std::ios::binary };
    if (stream)
    {
        try
        {
            pugi::xml_document doc;
            if (doc.load(stream)) {
                auto const sources = doc.child("source_list");
                std::scoped_lock lock{ m_mutex };
                for (auto const& s : sources.children("source"))
                {
                    m_package_sources.push_back(
                        source{
                            to_u8string(s.attribute("url").as_string()),
                            to_u8string(s.attribute("guid").as_string()), false });
                }
            }
        }
        catch (windower_error const&)
        {
            core::error(u8"package manager");
        }
    }
}

void windower::package_manager::save_source_list()
{
    pugi::xml_document doc;
    auto sources = doc.append_child("source_list");
    {
        std::scoped_lock lock{ m_mutex };
        for (auto const& s : m_package_sources)
        {
            if (!s.built_in)
            {
                auto source = sources.append_child("source");
                source.append_attribute("url").set_value(
                    reinterpret_cast<char const*>(s.url.c_str()));
                source.append_attribute("guid").set_value(
                    reinterpret_cast<char const*>(s.guid.c_str()));
            }
        }
    }
    std::ofstream stream{
        settings_path() / u8"updates" / u8"source_list", std::ios::binary };
    doc.save(stream);
}

std::future<void> windower::package_manager::update_sources(bool force)
{
    namespace fs = std::filesystem;
    auto sources_root = settings_path() / u8"updates";
    auto staging_path = sources_root / u8"staging";
    try { fs::remove_all(staging_path); fs::create_directories(staging_path); }
    catch (fs::filesystem_error const&) {}

    std::vector<downloader::file> files;
    for (auto const& source : m_package_sources)
    {
        auto url{ source.url };
        auto path = staging_path / source.guid;
        if (url.empty() || url.back() != u8'/') url.append(1, u8'/');
        url.append(u8"packages.xml");
        files.push_back({ url, path, last_modified(sources_root / source.guid, force) });
    }

    auto results = co_await core::instance().downloader.download(files);

    std::lock_guard<std::mutex> lock{ m_mutex };
    for (auto const& res : results)
    {
        auto path = sources_root / fs::relative(res.file().path, staging_path);
        if (::check(res) && res.status() == downloader::status::complete)
        {
            try
            {
                fs::create_directories(path.parent_path());
                fs::rename(res.file().path, path);
            }
            catch (fs::filesystem_error const&)
            {
                core::error(u8"package manager");
            }
        }
    }
}

std::future<void>
windower::package_manager::update_source(source source, bool force)
{
    namespace fs = std::filesystem;
    auto sources_root = settings_path() / u8"updates";
    auto staging_path = sources_root / u8"staging";
    try { fs::remove_all(staging_path); fs::create_directories(staging_path); }
    catch (fs::filesystem_error const&) {}

    std::vector<downloader::file> files;
    {
        auto url{ source.url };
        auto path = staging_path / source.guid;
        if (url.empty() || url.back() != u8'/') url.append(1, u8'/');
        url.append(u8"packages.xml");
        files.push_back({ url, path, last_modified(sources_root / source.guid, force) });
    }

    auto results = co_await core::instance().downloader.download(files);

    for (auto const& res : results)
    {
        auto path = sources_root / fs::relative(res.file().path, staging_path);
        if (::check(res) && res.status() == downloader::status::complete)
        {
            try
            {
                fs::create_directories(path.parent_path());
                fs::rename(res.file().path, path);
            }
            catch (fs::filesystem_error const&)
            {
                core::error(u8"package manager");
            }
        }
    }
}

void windower::package_manager::reload_sources()
{
    auto sources_root = settings_path() / u8"updates";
    std::lock_guard<std::mutex> lock{ m_mutex };
    m_available_packages.clear();
    for (auto const& source : m_package_sources)
    {
        load_source(sources_root / source.guid, source.url);
    }
}

void windower::package_manager::load_source(
    std::filesystem::path const& path, std::u8string const& url)
{
    auto stream = std::ifstream{ path, std::ios::binary };
    if (stream)
    {
        pugi::xml_document doc;
        if (!doc.load(stream)) return;

        auto const packages = doc.child("packages");
        if (!packages)
        {
            core::error(u8"package manager", u8"invalid source manifest");
            return;
        }

        for (auto const& package : packages.children("package"))
        {
            auto name = to_u8string(package.child_value("name"));
            auto it = std::lower_bound(
                m_available_packages.begin(), m_available_packages.end(), name,
                [](auto const& a, auto const& b) noexcept {
                    return a.name < b;
                });
            if (it == m_available_packages.end() || it->name != name)
            {
                auto& d = *m_available_packages.emplace(it);
                d.name = std::move(name);
                d.version = package_version{
                    package.child("version").text().as_string() };
                if (auto const dependencies = package.child("dependencies"))
                {
                    for (auto const& dependency : dependencies.children("dependency"))
                    {
                        d.dependencies.push_back(to_u8string(dependency.text().as_string()));
                    }
                }
                d.root_url = to_u8string(package.child("url").text().as_string(
                    reinterpret_cast<char const*>(url.c_str())));
                if (auto const files = package.child("files"))
                {
                    for (auto const& file : files.children("file"))
                    {
                        d.files.emplace_back(to_u8string(file.text().as_string()));
                    }
                }
            }
        }
    }
}

std::vector<windower::package_manager::descriptor>
windower::package_manager::out_of_date(
    std::vector<std::u8string> const& names) const
{
    std::vector<descriptor> results;
    auto unprocessed = std::vector<std::u8string>{ names.rbegin(), names.rend() };

    while (!unprocessed.empty())
    {
        auto const name = std::move(unprocessed.back());
        unprocessed.pop_back();

        auto const installed = m_installed_packages.find(name);
        auto const available = std::lower_bound(
            m_available_packages.begin(), m_available_packages.end(), name,
            [](auto const& a, auto const& b) noexcept { return a.name < b; });

        if (available == m_available_packages.end() || available->name != name)
        {
            if (installed == m_installed_packages.end())
            {
                core::error(
                    u8"package manager",
                    u8"could not find requested package or dependency \"" + name + u8"\"");
            }
            continue;
        }

        if (installed == m_installed_packages.end() ||
            installed->second.value->version() < available->version &&
            installed->second.value->can_update())
        {
            results.push_back(*available);
        }

        for (auto const& dependency : available->dependencies)
        {
            unprocessed.push_back(dependency);
        }
    }
    return results;
}

std::future<std::vector<std::u8string>>
windower::package_manager::install_or_update(
    std::vector<std::u8string> names, bool force)
{
    namespace fs = std::filesystem;
    std::vector<std::u8string> modified_packages;

    while (!names.empty())
    {
        std::vector<descriptor> packages;
        {
            std::lock_guard<std::mutex> lock{ m_mutex };
            packages = out_of_date(names);
        }

        if (packages.empty()) co_return std::vector<std::u8string>{};

        auto staging_path = settings_path() / u8"updates" / u8"temp";
        std::vector<downloader::job> jobs;

        for (auto const& p : packages)
        {
            auto root_url{ p.root_url };
            if (root_url.empty() || root_url.back() != u8'/') root_url.append(1, u8'/');

            std::vector<downloader::file> files;
            for (auto const& file : p.files)
            {
                auto url = root_url + file.generic_u8string();
                auto path = staging_path / file;
                path.make_preferred();
                fs::create_directories(path.parent_path());
                auto const time = last_modified(m_installed_package_directory / file, force);
                files.push_back({ url, path, time });
            }
            jobs.push_back(core::instance().downloader.download(files));
        }

        auto const count = std::min(jobs.size(), packages.size());
        for (std::size_t i = 0; i < count; ++i)
        {
            // CAVEMAN FIX: Silencing Bounds.4 warning by using gsl::at()
            auto results = co_await gsl::at(jobs, i);
            auto const& pkg = gsl::at(packages, i);

            if (!results.empty())
            {
                if (!std::all_of(
                    results.begin(), results.end(),
                    [](auto const& r) noexcept { return bool(r); }))
                {
                    auto message = u8"a problem occurred while installing \"" + pkg.name + u8"\"";
                    if (windower::core::instance().settings.verbose_logging)
                    {
                        for (auto const& r : results) ::check(r);
                    }
                }
                else
                {
                    std::vector<fs::path> old_files;
                    {
                        auto path = relative(results.front().file().path, staging_path);
                        if (path.has_parent_path())
                        {
                            auto root = m_installed_package_directory / *path.begin();
                            if (fs::exists(root))
                            {
                                old_files.insert(
                                    old_files.end(),
                                    fs::recursive_directory_iterator{ root },
                                    fs::recursive_directory_iterator{});
                            }
                        }
                    }

                    std::lock_guard<std::mutex> lock{ m_mutex };
                    for (auto const& r : results)
                    {
                        auto path = m_installed_package_directory / relative(r.file().path, staging_path);
                        auto it = std::find(old_files.begin(), old_files.end(), path);
                        if (it != old_files.end()) old_files.erase(it);

                        if (r.status() == downloader::status::complete)
                        {
                            try
                            {
                                fs::create_directories(path.parent_path());
                                fs::rename(r.file().path, path);
                            }
                            catch (fs::filesystem_error const&)
                            {
                                core::error(u8"package manager");
                            }
                        }
                    }

                    for (auto const& f : old_files)
                    {
                        if (!fs::is_directory(f)) fs::remove(f);
                    }
                    for (auto const& f : old_files)
                    {
                        if (fs::is_directory(f) && fs::is_empty(f)) fs::remove(f);
                    }
                }
            }
        }

        reset();
        names.clear();
        for (auto const& p : packages)
        {
            modified_packages.push_back(p.name);
            if (auto package = get_package(p.name))
            {
                for (auto const& d : package->dependencies())
                {
                    if (d.required() && !get_package(d.name())) names.push_back(d.name());
                }
            }
        }
    }
    co_return modified_packages;
}
