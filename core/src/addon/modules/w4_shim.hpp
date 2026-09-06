#pragma once
struct lua_State;

namespace windower::addon::w4_shim {
    void register_bindings(lua_State* L);
}
