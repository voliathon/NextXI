#include "addon/addon_manager.hpp"

#include "addon/addon.hpp"
#include "core.hpp"

#include <algorithm>
#include <iterator>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

windower::addon_manager::~addon_manager() noexcept { unload_all(); }

windower::addon const*
windower::addon_manager::get(std::u8string_view name) const
{
    std::lock_guard<std::mutex> lock{ m_mutex }; // thread-safe
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

    std::vector<std::shared_ptr<package const>> to_unload;
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        auto unload_order = package_manager->unload_order(names);
        std::copy_if(
            unload_order.begin(), unload_order.end(),
            std::back_inserter(to_unload), [=](auto p) {
                auto it = std::find_if(
                    m_loaded_addons.begin(), m_loaded_addons.end(),
                    [=](auto const& addon) {
                        return addon->package()->name() == p->name();
                    });
                return it != m_loaded_addons.end();
            });
    }
    unload(to_unload);

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
        std::lock_guard<std::mutex> lock{m_mutex};
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
    std::vector<gsl::not_null<addon*>> snapshot; // <-- Enforce not_null here

    // Safely acquire the snapshot
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
        // Verify the addon wasn't unloaded by a previous addon's run_until_idle()
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
            catch (...)
            {
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
    unload({name});
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
                    m_loaded_addons.emplace_back(std::move(ptr));
                }
            }
        }
    }
    catch (std::exception const& e)
    {
        core::error(u8"addon manager", u8"Error loading addon: " + windower::to_u8string(e.what()));

        bool needs_purge = false;

        // Scope the lock so it releases before the purge
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
                    m_loaded_addons.erase(it);
                    needs_purge = true;
                }
            }
        }

        if (needs_purge)
        {
            command_manager::instance().purge();
        }

        throw;
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
                m_loaded_addons.erase(it);
                needs_purge = true;
                core::output(u8"", package->name() + u8" unloaded");
            }
        }
    }

    // Purge exactly once outside the lock for maximum performance
    if (needs_purge)
    {
        command_manager::instance().purge();
    }
}
