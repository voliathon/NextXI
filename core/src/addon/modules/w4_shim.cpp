#include "w4_shim.hpp"
#include "packets/player_state.hpp"
#include <lua.hpp>

namespace windower::addon::w4_shim {

    static int lua_get_player(lua_State* L) {
        auto data = network::player_state_manager::get().get_data();

        lua_newtable(L);

        // Base stats W4 expects
        lua_pushinteger(L, data.hp); lua_setfield(L, -2, "hp");
        lua_pushinteger(L, data.max_hp); lua_setfield(L, -2, "max_hp");
        lua_pushinteger(L, data.mp); lua_setfield(L, -2, "mp");
        lua_pushinteger(L, data.max_mp); lua_setfield(L, -2, "max_mp");
        lua_pushinteger(L, data.id); lua_setfield(L, -2, "id");
        lua_pushinteger(L, data.index); lua_setfield(L, -2, "index");

        // W4 'vitals' sub-table
        lua_newtable(L);
        lua_pushinteger(L, data.hp); lua_setfield(L, -2, "hp");
        lua_pushinteger(L, data.max_hp); lua_setfield(L, -2, "max_hp");
        lua_pushinteger(L, data.mp); lua_setfield(L, -2, "mp");
        lua_pushinteger(L, data.max_mp); lua_setfield(L, -2, "max_mp");
        lua_pushinteger(L, data.tp); lua_setfield(L, -2, "tp");
        lua_setfield(L, -2, "vitals");

        // W4 position
        lua_pushnumber(L, data.x); lua_setfield(L, -2, "x");
        lua_pushnumber(L, data.y); lua_setfield(L, -2, "y");
        lua_pushnumber(L, data.z); lua_setfield(L, -2, "z");

        return 1; // Return table
    }

    void register_bindings(lua_State* L) {
        // Build windower.ffxi table tree if missing
        lua_getglobal(L, "windower");
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1); // FIXED
            lua_newtable(L);
            lua_pushvalue(L, -1);
            lua_setglobal(L, "windower");
        }

        lua_getfield(L, -1, "ffxi");
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1); // FIXED
            lua_newtable(L);
            lua_pushvalue(L, -1);
            lua_setfield(L, -3, "ffxi");
        }

        // Bind get_player
        lua_pushcfunction(L, lua_get_player);
        lua_setfield(L, -2, "get_player");

        lua_pop(L, 2); // FIXED
    }
}
