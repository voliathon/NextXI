#include "addon/modules/class.hpp"

#include "addon/lua.hpp"
#include "addon/modules/class.lua.hpp"

#include <lua.hpp>

int windower::load_class_module(lua::state s)
{
    lua::stack_guard guard{s};

    lua::load(guard, lua_class_source, u8"core.class");
    lua::call(guard, 0);

    return guard.release();
}