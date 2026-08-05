#include "addon/modules/pin.hpp"

#include "addon/lua.hpp"
#include "addon/modules/pin.lua.hpp"

int windower::load_pin_module(lua::state s)
{
    lua::stack_guard guard{s};

    lua::load(guard, lua_pin_source, u8"core.pin");
    lua::call(guard, 0);

    return guard.release();
}