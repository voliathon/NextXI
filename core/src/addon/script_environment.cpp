#include "addon/script_environment.hpp"

#include "addon/error.hpp"
#include "addon/lua.hpp"
#include "addon/lua_internal.hpp"
#include "addon/package_manager.hpp"
#include "addon/scheduler.hpp"

#include "addon/modules/command.hpp"

#include "core.hpp"
#include "utility.hpp"
#include "utilities/paths.hpp"

#include <filesystem>
#include <fstream>
#include <utility>

namespace
{

int load_script_module(windower::lua::state s)
{
    using namespace windower;

    auto name = lua::get<std::u8string>(s, 1);
    std::replace(name.begin(), name.end(), u8'.', u8'\\');
    name.append(u8".lua");

    auto path = windower_path() / u8"scripts" / name;
    std::ifstream stream{path, std::ios::binary};

    lua::stack_guard guard{s};

    if (stream.is_open())
    {
        try
        {
            lua::load(guard, stream, u8'@' + path.u8string());
        }
        catch (windower::lua::error const& e)
        {
            lua::push(
                guard, u8"\n    error loading module '" + name + u8"':\n\t" +
                           to_u8string(e.what()));
        }
    }
    else
    {
        lua::push(guard, u8"\n    no file '" + path.u8string() + u8'\'');
    }

    return gsl::narrow_cast<int>(guard.release());
}

}

void windower::script_environment::reset()
{
    script_base::reset();
    initialize();
}

std::shared_ptr<windower::package const>
windower::script_environment::find_dependency(
    lua::state, std::u8string_view package_name) const
{
    return core::instance().package_manager->get_package(package_name);
}

windower::script_environment::script_environment() noexcept
{
    initialize();
    m_scheduler.error_handler([=](std::exception_ptr exception, void const*) {
        core::error(u8"<script>", exception);
        return true;
    });
}

void windower::script_environment::initialize() const
{
    lua::stack_guard guard{ m_interpreter };

    lua::push(guard, u8"package");
    lua::raw_get(guard, lua::globals);
    lua::push(guard, u8"loaders");
    lua::raw_get(guard, -2);

    lua::push(guard, load_script_module);
    lua::raw_set(guard, -2, 2);
    lua::preload(m_interpreter, u8"core.command", &load_command_module);
}

void windower::script_environment::run_until_idle()
{
    script_base::run_until_idle();
}

void windower::script_environment::execute(std::u8string_view name) const
{
    auto path = windower_path() / u8"scripts" / name;
    path += u8".lua";
    std::ifstream stream{path, std::ios::binary};
    if (stream.is_open())
    {
        lua::stack_guard guard{m_interpreter};
        lua::load(guard, stream, u8'@' + path.u8string());
        lua::call(guard, 0);
    }
    else
    {
        throw lua::error{"no file '" + path.string() + '\''};
    }
}

void windower::script_environment::evaluate(std::u8string_view string) const
{
    lua::stack_guard guard{m_interpreter};
    lua::load(guard, string);
    lua::call(guard, 0);
}
