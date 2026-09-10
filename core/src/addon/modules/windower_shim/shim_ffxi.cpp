#include "shim.hpp"
#include "../packets/player_state.hpp"
#include <lua.hpp>

namespace windower::w4_shim {

    static int lua_get_player(lua_State* L) {
        auto const data = network::player_state_manager::get().get_data();
        lua_newtable(L);
        lua_pushinteger(L, data.hp); lua_setfield(L, -2, "hp");
        lua_pushinteger(L, data.max_hp); lua_setfield(L, -2, "max_hp");
        lua_pushinteger(L, data.mp); lua_setfield(L, -2, "mp");
        lua_pushinteger(L, data.max_mp); lua_setfield(L, -2, "max_mp");
        lua_pushinteger(L, data.id); lua_setfield(L, -2, "id");
        lua_pushinteger(L, data.index); lua_setfield(L, -2, "index");

        lua_newtable(L);
        lua_pushinteger(L, data.hp); lua_setfield(L, -2, "hp");
        lua_pushinteger(L, data.max_hp); lua_setfield(L, -2, "max_hp");
        lua_pushinteger(L, data.mp); lua_setfield(L, -2, "mp");
        lua_pushinteger(L, data.max_mp); lua_setfield(L, -2, "max_mp");
        lua_pushinteger(L, data.tp); lua_setfield(L, -2, "tp");
        lua_setfield(L, -2, "vitals");

        lua_pushnumber(L, data.x); lua_setfield(L, -2, "x");
        lua_pushnumber(L, data.y); lua_setfield(L, -2, "y");
        lua_pushnumber(L, data.z); lua_setfield(L, -2, "z");
        return 1;
    }

    void register_ffxi_bindings(lua_State* L) {
        lua_getfield(L, -1, "ffxi");
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            lua_newtable(L);
            lua_pushvalue(L, -1);
            lua_setfield(L, -3, "ffxi");
        }

        lua_pushcfunction(L, lua_get_player);
        lua_setfield(L, -2, "get_player");

        lua_pop(L, 1);
    }
}
