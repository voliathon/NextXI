#include "addon/addon_manager.hpp"

#include "addon/addon.hpp"
#include "core.hpp"
#include "errors/command_error.hpp"
#include "command_manager.hpp"

#include <algorithm>
#include <iterator>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>
#include <tuple>

windower::addon_manager::~addon_manager() noexcept { unload_all(); }

windower::addon const*
windower::addon_manager::get(std::u8string_view name) const
{
    std::lock_guard<std::mutex> lock{ m_mutex };
    auto it = std::find_if(
        m_loaded_addons.begin(), m_loaded_addons.end(),
        [&name](auto const& addon) {
            return addon->package()->name() == name;
        });
    return it == m_loaded_addons.end() ? nullptr : it->get();
}

std::vector<std::unique_ptr<windower::addon>> const&
windower::addon_manager::loaded() const noexcept
{
    return m_loaded_addons;
}

void windower::addon_manager::load(std::vector<std::u8string> const& names)
{
    load(core::instance().package_manager->load_order(names));
}

void windower::addon_manager::unload(std::vector<std::u8string> const& names)
{
    unload(core::instance().package_manager->unload_order(names));
}

void windower::addon_manager::reload(std::vector<std::u8string> const& names)
{
    auto const& package_manager = core::instance().package_manager;

    // 1. Ask the package manager for the dependency tree of what we want to unload
    auto const full_unload_order = package_manager->unload_order(names);

    // 2. Filter down to ONLY the packages that are ACTUALLY loaded right now.
    std::vector<std::shared_ptr<package const>> to_unload;
    {
        std::lock_guard<std::mutex> lock{ m_mutex };
        for (auto const& p : full_unload_order)
        {
            auto it = std::find_if(
                m_loaded_addons.begin(), m_loaded_addons.end(),
                [&p](auto const& addon) {
                    return addon->package()->name() == p->name();
                });

            if (it != m_loaded_addons.end())
            {
                to_unload.push_back(p);
            }
        }
    }

    // 3. Force the unload (teardown Lua sandbox, unhook commands, delete memory)
    if (!to_unload.empty())
    {
        unload(to_unload);
    }

    // 4. Force a clean rebuild!
    load(package_manager->load_order(names));
}

void windower::addon_manager::unload_all() noexcept
{
    unload(core::instance().package_manager->unload_order());
}

void windower::addon_manager::reload_all()
{
    std::vector<std::u8string> names;
    {
        std::lock_guard<std::mutex> lock{ m_mutex };
        std::transform(
            m_loaded_addons.begin(), m_loaded_addons.end(),
            std::back_inserter(names),
            [](auto const& a) { return a->package()->name(); });
    }
    reload(names);
}

void windower::addon_manager::run_until_idle()
{
    std::vector<std::u8string> failed_addons;
    std::vector<gsl::not_null<addon*>> snapshot;

    {
        std::lock_guard<std::mutex> lock{ m_mutex };
        for (auto const& a : m_loaded_addons)
        {
            if (a)
            {
                snapshot.push_back(gsl::make_not_null(a.get()));
            }
        }
    }

    for (auto const& a : snapshot)
    {
        bool exists = false;
        {
            std::lock_guard<std::mutex> lock{ m_mutex };
            exists = std::any_of(m_loaded_addons.begin(), m_loaded_addons.end(),
                [a](auto const& ptr) { return ptr.get() == a.get(); });
        }

        if (exists)
        {
            try
            {
                a->run_until_idle();
            }
            catch (std::exception const& e)
            {
                std::u8string const error_str = reinterpret_cast<char8_t const*>(e.what());
                core::instance().output(a->package()->name() + u8" [LUA EXCEPTION]", error_str);

                core::error(a->package()->name(), std::current_exception());
                failed_addons.push_back(a->package()->name());
            }
            catch (...)
            {
                core::instance().output(a->package()->name() + u8" [FATAL EXCEPTION]", u8"<UNKNOWN C++ ERROR>");
                core::error(a->package()->name(), std::current_exception());
                failed_addons.push_back(a->package()->name());
            }
        }
    }

    if (!failed_addons.empty())
    {
        unload(failed_addons);
    }
}

void windower::addon_manager::raise_error(
    gsl::not_null<package const*> package, std::exception_ptr exception)
{
    auto const& name = package->name();
    core::error(name, exception);

    core::instance().run_on_next_frame([name]() {
        core::instance().addon_manager->unload({ name });
        });
}

void windower::addon_manager::load(
    std::vector<std::shared_ptr<package const>> const& packages)
{
    std::vector<std::u8string> loaded_in_transaction;

    try
    {
        for (auto const& package : packages)
        {
            if (package->type() != package_type::library)
            {
                std::lock_guard<std::mutex> lock{ m_mutex };
                auto it = std::find_if(
                    m_loaded_addons.begin(), m_loaded_addons.end(),
                    [=](auto const& addon) {
                        return addon->package()->name() == package->name();
                    });

                if (it == m_loaded_addons.end())
                {
                    auto ptr = std::make_unique<addon>(package);

                    core::output(u8"", ptr->package()->name() + u8" loaded");
                    loaded_in_transaction.push_back(ptr->package()->name());
                    std::ignore = m_loaded_addons.emplace_back(std::move(ptr));
                }
            }
        }
    }
    catch (std::exception const& e)
    {
        std::u8string const error_str = reinterpret_cast<char8_t const*>(e.what());
        core::instance().output(u8"addon manager [LOAD EXCEPTION]", error_str);

        core::error(u8"addon manager", e);

        bool needs_purge = false;

        {
            std::lock_guard<std::mutex> lock{ m_mutex };
            for (auto const& name : loaded_in_transaction)
            {
                auto it = std::find_if(
                    m_loaded_addons.begin(), m_loaded_addons.end(),
                    [&name](auto const& addon) {
                        return addon->package()->name() == name;
                    });
                if (it != m_loaded_addons.end())
                {
                    core::output(u8"", name + u8" aborted");
                    std::ignore = m_loaded_addons.erase(it);
                    needs_purge = true;
                }
            }
        }

        if (needs_purge)
        {
            command_manager::instance().purge();
        }

        std::throw_with_nested(windower::command_error{ u8"ADDON_LOAD_FAILED", u8"addon_manager::load failed" });
    }
}

void windower::addon_manager::unload(
    std::vector<std::shared_ptr<package const>> const& packages)
{
    bool needs_purge = false;

    for (auto const& package : packages)
    {
        if (package->type() != package_type::library)
        {
            std::lock_guard<std::mutex> lock{ m_mutex };
            auto it = std::find_if(
                m_loaded_addons.begin(), m_loaded_addons.end(),
                [=](auto const& addon) {
                    return addon->package()->name() == package->name();
                });

            if (it != m_loaded_addons.end())
            {
                std::ignore = m_loaded_addons.erase(it);
                needs_purge = true;
                core::output(u8"", package->name() + u8" unloaded");
            }
        }
    }

    if (needs_purge)
    {
        command_manager::instance().purge();
    }
}
