#include "core.hpp"
#include <imgui.h>
#include <lua.hpp> // Ensure this points to your LuaJIT headers

namespace windower::lua
{
    // window_open = windower.imgui.begin_window("My Addon UI")
    static int lua_imgui_begin(lua_State* L)
    {
        const char* name = luaL_checkstring(L, 1);
        bool open = true;
        ImGui::Begin(name, &open);
        lua_pushboolean(L, open);
        return 1; // Returns true if the window is open
    }

    // windower.imgui.end_window()
    static int lua_imgui_end(lua_State* L)
    {
        ImGui::End();
        return 0;
    }

    // windower.imgui.text("HP: 1000/1000")
    static int lua_imgui_text(lua_State* L)
    {
        const char* text = luaL_checkstring(L, 1);
        ImGui::TextUnformatted(text);
        return 0;
    }

    // clicked = windower.imgui.button("Click Me")
    static int lua_imgui_button(lua_State* L)
    {
        const char* label = luaL_checkstring(L, 1);
        bool clicked = ImGui::Button(label);
        lua_pushboolean(L, clicked);
        return 1; // Returns true if the button was clicked this frame
    }

    // Register the functions into the windower.imgui table
    static const luaL_Reg imgui_funcs[] = {
        {"begin_window", lua_imgui_begin},
        {"end_window", lua_imgui_end},
        {"text", lua_imgui_text},
        {"button", lua_imgui_button},
        {nullptr, nullptr}
    };

    void register_imgui(lua_State* L)
    {
        lua_getglobal(L, "windower");
        if (lua_istable(L, -1))
        {
            lua_newtable(L);
            luaL_setfuncs(L, imgui_funcs, 0);
            lua_setfield(L, -2, "imgui");
        }
        lua_pop(L, 1);
    }
}
