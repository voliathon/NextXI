#include "addon/modules/serializer.hpp"

#include "addon/lua.hpp"
#include "addon/modules/serializer.lua.hpp"

#include <lua.hpp>

int windower::load_serializer_module(lua::state s)
{
    lua::stack_guard guard{s};

    lua::load(guard, lua_serializer_source, u8"core.serializer");
    lua::call(guard, 0);

    return guard.release();
}