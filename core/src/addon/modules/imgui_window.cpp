#include "addon/modules/imgui_internal.hpp"
#include "addon/lua.hpp"
#include "addon/unsafe.hpp" 
#include <lua.hpp>          
#include <imgui.h>

namespace windower::imgui_bindings
{
    namespace
    {
        int lua_imgui_begin_window(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* name = ::lua_tolstring(L, 1, nullptr);
            int flags = static_cast<int>(::lua_tointeger(L, 2));
            bool open = true;
            ImGui::Begin(name ? name : "Unnamed Window", &open, flags);
            ::lua_pushboolean(L, open ? 1 : 0);
            return 1;
        }

        int lua_imgui_end_window(lua::state s) {
            ImGui::End();
            return 0;
        }

        int lua_imgui_set_next_window_size(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            float w = static_cast<float>(::lua_tonumber(L, 1));
            float h = static_cast<float>(::lua_tonumber(L, 2));
            ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_FirstUseEver);
            return 0;
        }

        int lua_imgui_begin_child(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* str_id = ::lua_tolstring(L, 1, nullptr);
            float w = static_cast<float>(::lua_tonumber(L, 2));
            float h = static_cast<float>(::lua_tonumber(L, 3));
            bool border = ::lua_toboolean(L, 4) != 0;
            ::lua_pushboolean(L, ImGui::BeginChild(str_id ? str_id : "child", ImVec2(w, h), border));
            return 1;
        }

        int lua_imgui_end_child(lua::state s) {
            ImGui::EndChild();
            return 0;
        }
    }

    void bind_window(lua::stack_guard& guard)
    {
        auto bind = [&](const char8_t* name, auto func) {
            lua::push(guard, name);
            lua::push(guard, func);
            lua::raw_set(guard, -3);
            };

        bind(u8"begin_window", lua_imgui_begin_window);
        bind(u8"end_window", lua_imgui_end_window);
        bind(u8"set_next_window_size", lua_imgui_set_next_window_size);
        bind(u8"begin_child", lua_imgui_begin_child);
        bind(u8"end_child", lua_imgui_end_child);
    }
}
