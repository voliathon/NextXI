#include "addon/modules/imgui_internal.hpp"
#include "addon/lua.hpp"
#include "addon/unsafe.hpp" 
#include <lua.hpp>          
#include <imgui.h>

namespace windower::imgui_bindings
{
    namespace
    {
        int lua_imgui_set_color(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            int idx = static_cast<int>(::lua_tointeger(L, 1));
            float r = static_cast<float>(::lua_tonumber(L, 2));
            float g = static_cast<float>(::lua_tonumber(L, 3));
            float b = static_cast<float>(::lua_tonumber(L, 4));
            float a = static_cast<float>(::lua_tonumber(L, 5));

            if (idx >= 0 && idx < ImGuiCol_COUNT) {
                ImGui::GetStyle().Colors[idx] = ImVec4(r, g, b, a);
            }
            return 0;
        }

        int lua_imgui_get_color(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            int idx = static_cast<int>(::lua_tointeger(L, 1));
            if (idx >= 0 && idx < ImGuiCol_COUNT) {
                ImVec4 const& c = ImGui::GetStyle().Colors[idx];
                ::lua_pushnumber(L, static_cast<double>(c.x));
                ::lua_pushnumber(L, static_cast<double>(c.y));
                ::lua_pushnumber(L, static_cast<double>(c.z));
                ::lua_pushnumber(L, static_cast<double>(c.w));
                return 4;
            }
            return 0;
        }

        int lua_imgui_set_rounding(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            float window_rounding = static_cast<float>(::lua_tonumber(L, 1));
            float frame_rounding = static_cast<float>(::lua_tonumber(L, 2));
            float popup_rounding = static_cast<float>(::lua_tonumber(L, 3));
            float tab_rounding = static_cast<float>(::lua_tonumber(L, 4));

            ImGuiStyle& style = ImGui::GetStyle();
            style.WindowRounding = window_rounding;
            style.FrameRounding = frame_rounding;
            style.PopupRounding = popup_rounding;
            style.TabRounding = tab_rounding;
            return 0;
        }

        int lua_imgui_reset_default_style(lua::state s) {
            ImGui::StyleColorsDark();
            return 0;
        }
    }

    void bind_style(lua::stack_guard& guard)
    {
        auto bind = [&](const char8_t* name, auto func) {
            lua::push(guard, name);
            lua::push(guard, func);
            lua::raw_set(guard, -3);
            };

        bind(u8"set_color", lua_imgui_set_color);
        bind(u8"get_color", lua_imgui_get_color);
        bind(u8"set_rounding", lua_imgui_set_rounding);
        bind(u8"reset_default_style", lua_imgui_reset_default_style);
    }
}
