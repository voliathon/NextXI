#pragma once

struct lua_State;

namespace windower::w4_shim {
    void register_bindings(lua_State* L);

    void register_chat_bindings(lua_State* L);
    void register_event_bindings(lua_State* L);
    void register_ffxi_bindings(lua_State* L);
}
