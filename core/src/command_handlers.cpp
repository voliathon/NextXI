#include "command_handlers.hpp"

#include "command_manager.hpp"
#include "core.hpp"
#include "errors/command_error.hpp"
#include "unicode.hpp"
#include "utility.hpp"
#include "utilities/module_info.hpp"

#include <limits>

namespace
{
    constexpr auto unlimited = std::numeric_limits<std::size_t>::max();

    // Helper: Validates argument counts for incoming user commands.
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

    void check_args(
        std::u8string_view command_name, std::vector<std::u8string> const& args,
        std::size_t expected)
    {
        check_args(command_name, args, expected, expected);
    }
} // namespace

// ==========================================
// ADDON LIFECYCLE COMMANDS
// ==========================================

void windower::command_handlers::load(
    std::vector<std::u8string> const& args, windower::command_source)
{
    check_args(u8"/load", args, 1, unlimited);
    auto const& core = core::instance();
    if (core.addon_manager)
    {
        core.addon_manager->load(args);
    }
    else
    {
        throw command_error{ u8"Addon manager is not initialized", u8"/load" };
    }
}

void windower::command_handlers::unload(
    std::vector<std::u8string> const& args, command_source)
{
    check_args(u8"/unload", args, 1, unlimited);
    auto const& core = core::instance();
    if (core.addon_manager)
    {
        core.addon_manager->unload(args);
    }
    else
    {
        throw command_error{ u8"Addon manager is not initialized", u8"/unload" };
    }
}

void windower::command_handlers::reload(
    std::vector<std::u8string> const& args, command_source)
{
    check_args(u8"/reload", args, 1, unlimited);
    auto const& core = core::instance();
    if (core.addon_manager)
    {
        core.addon_manager->reload(args);
    }
    else
    {
        throw command_error{ u8"Addon manager is not initialized", u8"/reload" };
    }
}

void windower::command_handlers::unloadall(
    std::vector<std::u8string> const& args, command_source)
{
    check_args(u8"/unloadall", args, 0);
    auto const& core = core::instance();
    if (core.addon_manager)
    {
        core.addon_manager->unload_all();
    }
    else
    {
        throw command_error{
            u8"Addon manager is not initialized", u8"/unloadall" };
    }
}

void windower::command_handlers::reloadall(
    std::vector<std::u8string> const& args, command_source)
{
    check_args(u8"/reloadall", args, 0);
    auto const& core = core::instance();
    if (core.addon_manager)
    {
        core.addon_manager->reload_all();
    }
    else
    {
        throw command_error{
            u8"Addon manager is not initialized", u8"/reloadall" };
    }
}

// ==========================================
// ALIAS & BINDING COMMANDS
// ==========================================

void windower::command_handlers::alias(
    std::vector<std::u8string> const& args, command_source)
{
    if (args.at(0).length() > 6)
    {
        auto parsed = command_manager::get_arguments(args.at(0).substr(7), 1);
        check_args(u8"/alias", parsed, 2);
        std::u8string_view command = gsl::at(parsed, 1);
        auto index = std::size_t{};
        auto next_index = index;
        while (is_whitespace(next_code_point(command, next_index)))
        {
            index = next_index;
        }
        command.remove_prefix(index);
        command_manager::instance().register_alias(gsl::at(parsed, 0), command);
    }
}

void windower::command_handlers::unalias(
    std::vector<std::u8string> const& args, command_source)
{
    check_args(u8"/unalias", args, 1);
    command_manager::instance().unregister_alias(gsl::at(args, 0));
}

void windower::command_handlers::bind(
    std::vector<std::u8string> const& args, command_source)
{
    if (args.at(0).length() > 5)
    {
        core::instance().binding_manager.bind(gsl::at(args, 0).substr(6));
    }
}

void windower::command_handlers::unbind(
    std::vector<std::u8string> const& args, command_source)
{
    if (args.at(0).length() > 7)
    {
        core::instance().binding_manager.unbind(gsl::at(args, 0).substr(8));
    }
}

void windower::command_handlers::listbinds(
    std::vector<std::u8string> const& args, command_source source)
{
    check_args(u8"/listbinds", args, 0);
    auto const& bindings = core::instance().binding_manager.get_binds();
    if (bindings.empty())
    {
        core::output(u8"core", u8"No key bindings registered.", source);
        return;
    }
    core::output(
        u8"core",
        u8"Total bindings: " + windower::to_u8string(bindings.size()));
    for (auto const& binding : bindings)
    {
        core::output(u8"core", binding.first + u8": " + binding.second, source);
    }
}

// ==========================================
// SCRIPTING & UI COMMANDS
// ==========================================

void windower::command_handlers::exec(
    std::vector<std::u8string> const& args, command_source)
{
    check_args(u8"/exec", args, 1);
    core::instance().script_environment.execute(gsl::at(args, 0));
}

void windower::command_handlers::eval(
    std::vector<std::u8string> const& args, command_source)
{
    auto arg = std::u8string_view{ args.at(0) };
    if (arg.length() > 5)
    {
        arg.remove_prefix(6);
        core::instance().script_environment.evaluate(arg);
    }
}

void windower::command_handlers::reset(
    std::vector<std::u8string> const& args, command_source)
{
    check_args(u8"/reset", args, 0);
    core::instance().script_environment.reset();
}

void windower::command_handlers::nextwindow(
    std::vector<std::u8string> const& args, command_source)
{
    check_args(u8"/nextwindow", args, 0);
    core::instance().ui.activate_next_window();
}

void windower::command_handlers::prevwindow(
    std::vector<std::u8string> const& args, command_source)
{
    check_args(u8"/prevwindow", args, 0);
    core::instance().ui.activate_previous_window();
}

void windower::command_handlers::register_all()
{
    auto& cmd = command_manager::instance();

    cmd.register_command(command_manager::layer::core, u8"", u8"install", install);
    cmd.register_command(command_manager::layer::core, u8"", u8"uninstall", uninstall);
    cmd.register_command(command_manager::layer::core, u8"", u8"update", update);
    cmd.register_command(command_manager::layer::core, u8"", u8"updateall", updateall);
    cmd.register_command(command_manager::layer::core, u8"", u8"load", load);
    cmd.register_command(command_manager::layer::core, u8"", u8"unload", unload);
    cmd.register_command(command_manager::layer::core, u8"", u8"reload", reload);
    cmd.register_command(command_manager::layer::core, u8"", u8"unloadall", unloadall);
    cmd.register_command(command_manager::layer::core, u8"", u8"reloadall", reloadall);
    cmd.register_command(command_manager::layer::core, u8"", u8"alias", alias, true);
    cmd.register_command(command_manager::layer::core, u8"", u8"unalias", unalias);
    cmd.register_command(command_manager::layer::core, u8"", u8"bind", bind, true);
    cmd.register_command(command_manager::layer::core, u8"", u8"unbind", unbind, true);
    cmd.register_command(command_manager::layer::core, u8"", u8"listbinds", listbinds, true);
    cmd.register_command(command_manager::layer::core, u8"", u8"exec", exec);
    cmd.register_command(command_manager::layer::core, u8"", u8"eval", eval, true);
    cmd.register_command(command_manager::layer::core, u8"", u8"reset", reset);
    cmd.register_command(command_manager::layer::core, u8"", u8"pkg", pkg);
    cmd.register_command(command_manager::layer::core, u8"", u8"nextwindow", nextwindow);
    cmd.register_command(command_manager::layer::core, u8"", u8"prevwindow", prevwindow);
}
