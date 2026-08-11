#include "command_manager.hpp"
#include "command_parser.hpp"

#include "addon/modules/command.hpp"
#include "errors/command_error.hpp"
#include "errors/syntax_error.hpp"
#include "errors/windower_error.hpp"
#include "hooks/ffximain.hpp"
#include "unicode.hpp"

#include "utilities/debug_helpers.hpp"

#include <cstddef>
#include <iterator>
#include <regex>
#include <string>
#include <tuple>
#include <vector>

namespace
{
    constexpr std::u8string_view name_chars =
        u8"abcdefghijklmnopqrstuvwxyz"
        u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        u8"0123456789?_-";
}

void windower::command_manager::initialize() noexcept { instance(); }

windower::command_manager& windower::command_manager::instance() noexcept
{
    static command_manager instance;
    return instance;
}

bool windower::command_manager::is_valid_name(std::u8string_view name) noexcept
{
    return name.find_first_not_of(name_chars) != std::u8string_view::npos;
}

void windower::command_manager::check_name(std::u8string_view name)
{
    if (auto const offset = name.find_first_not_of(name_chars);
        offset != std::u8string_view::npos)
    {
        throw syntax_error{u8"CMD:N1", name, offset};
    }
}

std::u8string windower::command_manager::escape(std::u8string_view string)
{
    std::u8string result;
    result.reserve(string.size());
    auto mark = std::size_t{};
    for (auto it = std::size_t{}, next_it = it; it != string.size();)
    {
        auto const c = next_code_point(string, next_it);
        if (c == U'\\' || c == U'\'' || c == U'"' || is_whitespace(c))
        {
            result.append(string.substr(mark, it - mark));
            result.append(1, u8'\\');
            mark = it;
        }
        it = next_it;
    }
    return result;
}

std::u8string windower::command_manager::unescape(std::u8string_view string)
{
    return command_parser::unescape(string);
}

std::vector<std::u8string> windower::command_manager::get_arguments(
    std::u8string_view string, std::size_t count)
{
    std::vector<std::u8string> results;
    command_parser::parse_arguments(string, count, results);
    return results;
}

void windower::command_manager::register_command(
    layer layer, std::u8string_view component, std::u8string_view command,
    std::function<void(std::vector<std::u8string>, command_source)> handler,
    bool raw, std::shared_ptr<void> const& tag)
{
    check_name(component);
    check_name(command);
    if (command.empty())
    {
        throw syntax_error{u8"CMD:R1"};
    }

    descriptor d;
    d.component = component;
    d.command   = command;
    d.tag       = tag ? tag : m_default_tag;
    d.handler   = std::move(handler);
    d.raw       = raw;
    d.layer     = layer;

    auto& commands = m_commands.at(gsl::narrow_cast<int>(layer));
    if (auto it = std::lower_bound(commands.begin(), commands.end(), d);
        it == commands.end() || d < *it)
    {
        commands.insert(it, std::move(d));
    }
    else if (it != commands.end())
    {
        *it = std::move(d);
    }
}

void windower::command_manager::unregister_command(
    layer layer, std::u8string_view component, std::u8string_view command)
{
    name_view const name{component, command};
    auto& commands = m_commands.at(gsl::narrow_cast<int>(layer));
    commands.erase(
        std::remove(commands.begin(), commands.end(), name), commands.end());
}

void windower::command_manager::register_alias(
    std::u8string_view alias, std::u8string_view command)
{
    check_name(alias);
    if (alias.empty())
    {
        throw syntax_error{u8"CMD:A1"};
    }
    validate_command(command);

    if (auto it = std::lower_bound(
            m_aliases.begin(), m_aliases.end(), alias,
            [](auto const& a, auto const& b) { return a.first < b; });
        it == m_aliases.end() || it->first != alias)
    {
        m_aliases.emplace(it, alias, command);
    }
    else if (it != m_aliases.end())
    {
        it->second = command;
    }
}

void windower::command_manager::unregister_alias(std::u8string_view alias)
{
    m_aliases.erase(
        std::remove_if(
            m_aliases.begin(), m_aliases.end(),
            [&](auto const& a) { return a.first == alias; }),
        m_aliases.end());
}

void windower::command_manager::validate_command(
    std::u8string_view const command_string) const
{
    command_parser::parse_command(command_string);
}

void windower::command_manager::handle_command(
    std::u8string_view command_string, command_source const source)
{
    std::u8string expanded;

    // CAVEMAN FIX: Intercept Windower 4 muscle memory before parsing!
    std::u8string legacy_fix;
    if (command_string.starts_with(u8"//lua "))
    {
        legacy_fix.append(u8"/");
        legacy_fix.append(command_string.substr(6));
        command_string = legacy_fix;
    }
    else if (command_string.starts_with(u8"//"))
    {
        legacy_fix.append(u8"/");
        legacy_fix.append(command_string.substr(2));
        command_string = legacy_fix;
    }

    auto [component, command] = command_parser::parse_command(command_string);

    if (!component)
    {
        if (auto alias = resolve_alias(command_parser::substring(command_string, command)))
        {
            expanded.append(*alias);
            expanded.append(command_string.substr(command.second));
            command_string    = expanded;
            auto const result = command_parser::parse_command(command_string);
            component         = result.first;
            command           = result.second;
        }
    }

    while (auto descriptor = find(command_string, component, command))
    {
        auto lock = descriptor->tag.lock();
        if (!lock)
        {
            unregister_command(
                descriptor->layer, descriptor->component, descriptor->command);
            continue;
        }
        std::vector<std::u8string> arguments;
        if (descriptor->raw)
        {
            std::u8string processed_command;
            processed_command.append(1, u8'/');
            processed_command.append(command_parser::substring(command_string, command));
            processed_command.append(command_string.substr(command.second));
            arguments.emplace_back(processed_command);
        }
        else
        {
            auto const arg_string = command_string.substr(command.second);
            command_parser::parse_arguments(arg_string, unlimited, arguments);
        }
        try
        {
            descriptor->handler(arguments, source);
            return;
        }
        catch (command_error const&)
        {
            throw;
        }
        catch (std::exception const& e) // Name the exception!
        {
            // Do not throw away the inner Lua error! Chain them together!
            std::throw_with_nested(command_error{ u8"CMD:X1", command_string });
        }
    }

    if (component || !trigger_unknown_command(command_string, source))
    {
        throw syntax_error{
            u8"CMD:L2", command_string, command.first, command.first,
            command.second};
    }
}

void windower::command_manager::purge() noexcept
{
    for (auto& commands : m_commands)
    {
        commands.erase(
            std::remove_if(
                commands.begin(), commands.end(),
                [](descriptor& d) noexcept { return d.tag.expired(); }),
            commands.end());
    }
}

std::optional<std::u8string_view>
windower::command_manager::resolve_alias(std::u8string_view name) const
{
    auto it = std::lower_bound(
        m_aliases.begin(), m_aliases.end(), name,
        [](auto const& a, auto const& b) { return a.first < b; });
    if (it != m_aliases.end() && it->first == name)
    {
        return it->second;
    }
    return std::nullopt;
}

windower::command_manager::descriptor const* windower::command_manager::find(
    std::u8string_view command_string, std::optional<u8range> component,
    u8range command) const
{
    name_view const name{
        command_parser::substring(command_string, component),
        command_parser::substring(command_string, command)};
    for (auto& commands : m_commands)
    {
        auto const bounds =
            std::equal_range(commands.begin(), commands.end(), name);
        auto const count = std::distance(bounds.first, bounds.second);
        if (count == 1)
        {
            return &*bounds.first;
        }
        if (count > 1 && !component)
        {
            std::vector<std::u8string> options;
            std::transform(
                bounds.first, bounds.second, std::back_inserter(options),
                [](auto const& d) {
                    return u8'/' + d.component + u8':' + d.command;
                });
            throw syntax_error{u8"CMD:L1",    command_string, command.first,
                               command.first, command.second, options};
        }
    }
    if (component)
    {
        throw syntax_error{
            u8"CMD:L2", command_string, component->first, component->first,
            command.second};
    }
    return nullptr;
}

bool windower::command_manager::descriptor::operator==(
    descriptor const& other) const noexcept
{
    return command == other.command && component == other.component;
}

std::strong_ordering windower::command_manager::descriptor::operator<=>(
    descriptor const& other) const noexcept
{
    auto const result = command.compare(other.command);

    if (result != 0)
    {
        return result <=> 0;
    }

    return component.compare(other.component) <=> 0;
}

windower::command_manager::name_view::name_view(
    descriptor const& descriptor) noexcept :
    component{descriptor.component},
    command{descriptor.command}
{}

windower::command_manager::name_view::name_view(
    std::u8string_view command) noexcept :
    component{std::nullopt},
    command{std::move(command)}
{}

windower::command_manager::name_view::name_view(
    std::optional<std::u8string_view> component,
    std::u8string_view command) noexcept :
    component{std::move(component)},
    command{std::move(command)}
{}

bool windower::command_manager::name_view::operator==(
    name_view const& other) const noexcept
{
    return command == other.command &&
           (!component || !other.component || *component == *other.component);
}

std::weak_ordering windower::command_manager::name_view::operator<=>(
    name_view const& other) const noexcept
{
    auto const result = command.compare(other.command);
    if (result != 0)
    {
        return result <=> 0;
    }
    if (!component || !other.component)
    {
        return std::weak_ordering::equivalent;
    }
    return component->compare(*other.component) <=> 0;
}
