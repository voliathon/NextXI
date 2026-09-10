#include "shim.hpp"
#include "core.hpp"
#include "hooks/ffximain.hpp"
#include <lua.hpp>
#include <string>

namespace windower::w4_shim {

    static int lua_add_to_chat(lua_State* L) {
        int color = static_cast<int>(luaL_optinteger(L, 1, 8));
        const char* text = luaL_optstring(L, 2, "");
        if (text) {
            std::u8string u8text(reinterpret_cast<const char8_t*>(text));
            core::instance().run_on_next_frame([u8text, color]() {
                windower::ffximain::add_to_chat(u8text, static_cast<uint8_t>(color), false);
                });
        }
        return 0;
    }

    void register_chat_bindings(lua_State* L) {
        lua_pushcfunction(L, lua_add_to_chat);
        lua_setfield(L, -2, "add_to_chat");
    }
}
