#include "command_handlers.hpp"

#include "addon/addon_manager.hpp"
#include "addon/package_manager.hpp"

#include "command_manager.hpp"
#include "core.hpp"
#include "errors/command_error.hpp"
#include "errors/windower_error.hpp"
#include "utilities/coroutine.hpp"
#include "utility.hpp"

#include <algorithm>
#include <limits>
#include <memory>
#include <string_view>
#include <vector>

namespace
{
    constexpr auto unlimited = std::numeric_limits<std::size_t>::max();

    // Helper: Validates that the command arguments fall within expected min/max bounds.
    void check_args(
        std::u8string_view command_name, std::vector<std::u8string> const& args,
        std::size_t min, std::size_t max)
    {
        using namespace windower;

        auto const count = args.size();
        if (count < min)
        {
            std::u8string message;
            message.append(u8"Too few arguments; expected: ");
            message.append(to_u8string(min));
            message.append(u8", got: ");
            message.append(to_u8string(count));

            throw command_error{ message, command_name };
        }
        else if (count > max)
        {
            std::u8string message;
            message.append(u8"Too many arguments; expected: ");
            message.append(to_u8string(min));
            message.append(u8", got: ");
            message.append(to_u8string(count));

            throw command_error{ message, command_name };
        }
    }

    // Helper: Validates exact argument count
    void check_args(
        std::u8string_view command_name, std::vector<std::u8string> const& args,
        std::size_t expected)
    {
        check_args(command_name, args, expected, expected);
    }

    // Coroutine Implementation: Asynchronously downloads and installs requested packages.
    std::future<void> install_impl(std::vector<std::u8string> const& args)
    {
        try
        {
            check_args(u8"/install", args, 1, unlimited);
            auto const& core = windower::core::instance();
            auto updated = co_await core.package_manager->install(args);
            if (core.addon_manager)
            {
                core.addon_manager->reload(updated);
            }
            windower::core::output(
                u8"System", u8"Packages successfully installed.");
        }
        catch (std::exception const& e)
        {
            windower::core::error(u8"Downloader", u8"Install failed: " + windower::to_u8string(e.what()));
        }
    }

    // Coroutine Implementation: Asynchronously updates specified packages.
    std::future<void> update_impl(std::vector<std::u8string> const& args)
    {
        try
        {
            check_args(u8"/update", args, 1, unlimited);
            auto const& core = windower::core::instance();
            auto updated = co_await core.package_manager->update(args);
            if (core.addon_manager)
            {
                core.addon_manager->reload(updated);
            }
            windower::core::output(u8"System", u8"Packages successfully updated.");
        }
        catch (std::exception const& e)
        {
            windower::core::error(u8"Downloader", u8"Update failed: " + windower::to_u8string(e.what()));
        }
    }

    // Coroutine Implementation: Asynchronously updates all installed packages.
    std::future<void> updateall_impl(std::vector<std::u8string> const& args)
    {
        try
        {
            check_args(u8"/updateall", args, 0, 1);
            auto const& core = windower::core::instance();
            auto const force = !args.empty() && gsl::at(args, 0) == u8"force";
            auto updated = co_await core.package_manager->update_all(force);
            if (core.addon_manager)
            {
                core.addon_manager->reload(updated);
            }
            windower::core::output(
                u8"System", u8"All packages successfully updated.");
        }
        catch (std::exception const& e)
        {
            windower::core::error(u8"Downloader", u8"Update All failed: " + windower::to_u8string(e.what()));
        }
    }
} // namespace

// Command Handler: /install <package_name>
void windower::command_handlers::install(
    std::vector<std::u8string> const& args, windower::command_source)
{
    install_impl(args);
}

// Command Handler: /uninstall <package_name>
void windower::command_handlers::uninstall(
    std::vector<std::u8string> const& args, windower::command_source)
{
    check_args(u8"/uninstall", args, 1, unlimited);

    auto const& core = core::instance();
    auto packages = core.package_manager->unload_order(args);
    std::vector<std::shared_ptr<windower::package const>> dependents;
    while (!packages.empty())
    {
        if (std::find(args.begin(), args.end(), packages.back()->name()) ==
            args.end())
        {
            dependents.push_back(packages.back());
        }
        packages.pop_back();
    }

    if (!dependents.empty())
    {
        std::u8string message = u8"Uninstall failed.\n The following ";
        message += dependents.size() == 1 ? u8"package depends on "
            : u8"packages depend on ";
        message += args.size() == 1 ? u8"this package:\n"
            : u8"one or more of these packages:\n";
        for (auto const& package : dependents)
        {
            message += u8"    ";
            message += package->name();
        }
        throw windower_error{ message };
    }

    if (core.addon_manager)
    {
        core.addon_manager->unload(args);
    }

    core.package_manager->uninstall(args);
}

// Command Handler: /update <package_name>
void windower::command_handlers::update(
    std::vector<std::u8string> const& args, windower::command_source)
{
    update_impl(args);
}

// Command Handler: /updateall [force]
void windower::command_handlers::updateall(
    std::vector<std::u8string> const& args, windower::command_source)
{
    updateall_impl(args);
}

// Command Handler: /pkg <reload|listsrc|addsrc|removesrc>
void windower::command_handlers::pkg(
    std::vector<std::u8string> const& args, command_source source)
{
    check_args(u8"/pkg", args, 1, unlimited);
    auto const& core = core::instance();
    if (gsl::at(args, 0) == u8"reload")
    {
        check_args(u8"/pkg", args, 1);
        core.package_manager->reset();
        windower::core::output(
            u8"System", u8"Package cache successfully reloaded.", source);
    }
    else if (gsl::at(args, 0) == u8"listsrc")
    {
        check_args(u8"/pkg", args, 1);
        core::output(u8"package manager", u8"listing sources...", source);
        for (auto const& s : core.package_manager->sources())
        {
            windower::core::output(u8"package manager", s, source);
        }
    }
    else if (gsl::at(args, 0) == u8"addsrc")
    {
        check_args(u8"/pkg", args, 2);
        core.package_manager->add_source(gsl::at(args, 1));
    }
    else if (gsl::at(args, 0) == u8"removesrc")
    {
        check_args(u8"/pkg", args, 2);
        core.package_manager->remove_source(gsl::at(args, 1));
    }
    else
    {
        std::u8string message;
        message.append(u8"Unrecognized package manager sub-command \"");
        message.append(gsl::at(args, 0));
        message.append(u8"\"");

        throw command_error{ message, u8"/pkg" };
    }
}
