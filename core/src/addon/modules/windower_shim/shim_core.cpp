#include "shim.hpp"
#include <lua.hpp>

namespace windower::w4_shim {

    void register_bindings(lua_State* L) {
        lua_getglobal(L, "windower");
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            lua_newtable(L);
            lua_pushvalue(L, -1);
            lua_setglobal(L, "windower");
        }

        register_chat_bindings(L);
        register_event_bindings(L);
        register_ffxi_bindings(L);

        lua_pop(L, 1);
    }
}
