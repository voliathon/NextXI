#include "addon/modules/hash.hpp"

#include "addon/lua.hpp"
#include "addon/modules/hash.lua.hpp"

#include <lua.hpp>

int windower::load_hash_module(lua::state s)
{
    lua::stack_guard guard{s};

    lua::load(guard, lua_hash_source, u8"core.hash");
    lua::call(guard, 0);

    return guard.release();
}