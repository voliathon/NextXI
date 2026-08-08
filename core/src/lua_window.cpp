#include "core.hpp"
#include "unicode.hpp"
#include <lua.hpp> // Ensure this points to your LuaJIT headers

namespace windower::lua
{
    // Lua usage: windower.set_window_title("Next XI - Voliathon")
    static int lua_set_window_title(lua_State* L)
    {
        // Grab the UTF-8 string from the Lua script
        const char* title_utf8 = luaL_checkstring(L, 1);

        // Convert it to Windows UTF-16
        auto title_utf16 = windower::to_wstring(std::u8string_view{ reinterpret_cast<const char8_t*>(title_utf8) });

        // Grab the active game window and update the title instantly!
        HWND hwnd = static_cast<HWND>(windower::core::instance().client_hwnd);
        if (hwnd)
        {
            ::SetWindowTextW(hwnd, title_utf16.c_str());
        }

        return 0;
    }

    // Register the function into the windower table
    static const luaL_Reg window_funcs[] = {
        {"set_window_title", lua_set_window_title},
        {nullptr, nullptr}
    };

    void register_window(lua_State* L)
    {
        lua_getglobal(L, "windower");
        if (lua_istable(L, -1))
        {
            // Silencing C26485 (bounds.3) by passing the explicit pointer
            luaL_setfuncs(L, &window_funcs[0], 0);
        }
        lua_pop(L, 1);
    }
}
