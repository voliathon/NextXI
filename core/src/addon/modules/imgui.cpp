#include "addon/modules/imgui.hpp"
#include "addon/lua.hpp"
#include "addon/unsafe.hpp" // Crucial for getting the raw lua_State*
#include <lua.hpp>          // Crucial for raw C-API exceptionless functions
#include <imgui.h>

namespace windower
{
    namespace
    {
        int lua_imgui_begin_window(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* name = ::lua_tolstring(L, 1, nullptr);
            if (!name) name = "Unnamed Window";

            bool open = true;
            ImGui::Begin(name, &open);

            ::lua_pushboolean(L, open ? 1 : 0);
            return 1;
        }

        int lua_imgui_end_window(lua::state s) {
            ImGui::End();
            return 0;
        }

        int lua_imgui_text(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* text = ::lua_tolstring(L, 1, nullptr);
            if (text) {
                ImGui::TextUnformatted(text);
            }
            return 0;
        }

        int lua_imgui_button(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* label = ::lua_tolstring(L, 1, nullptr);
            if (!label) label = "Button";

            bool clicked = ImGui::Button(label);

            ::lua_pushboolean(L, clicked ? 1 : 0);
            return 1;
        }

        int lua_imgui_separator(lua::state s) {
            ImGui::Separator();
            return 0;
        }

        int lua_imgui_same_line(lua::state s) {
            ImGui::SameLine();
            return 0;
        }

        int lua_imgui_checkbox(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* label = ::lua_tolstring(L, 1, nullptr);
            if (!label) label = "Checkbox";

            bool checked = ::lua_toboolean(L, 2) != 0;
            ImGui::Checkbox(label, &checked);

            ::lua_pushboolean(L, checked ? 1 : 0);
            return 1;
        }

        // ====================================================================
        // SANDBOX TOOLS - PURE C-API (ZERO C++ EXCEPTIONS)
        // If Lua passes garbage (nils, strings instead of numbers), these 
        // safely fallback to 0.0 without detonating the engine!
        // ====================================================================

        int lua_imgui_slider_float(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* label = ::lua_tolstring(L, 1, nullptr);
            if (!label) label = "SliderFloat";

            float v = static_cast<float>(::lua_tonumber(L, 2));
            float min = static_cast<float>(::lua_tonumber(L, 3));
            float max = static_cast<float>(::lua_tonumber(L, 4));

            ImGui::SliderFloat(label, &v, min, max);

            ::lua_pushnumber(L, static_cast<double>(v));
            return 1;
        }

        int lua_imgui_slider_int(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* label = ::lua_tolstring(L, 1, nullptr);
            if (!label) label = "SliderInt";

            int v = static_cast<int>(::lua_tointeger(L, 2));
            int min = static_cast<int>(::lua_tointeger(L, 3));
            int max = static_cast<int>(::lua_tointeger(L, 4));

            ImGui::SliderInt(label, &v, min, max);

            ::lua_pushinteger(L, v);
            return 1;
        }

        int lua_imgui_color_edit3(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* label = ::lua_tolstring(L, 1, nullptr);
            if (!label) label = "ColorEdit";

            float col[3] = {
                static_cast<float>(::lua_tonumber(L, 2)),
                static_cast<float>(::lua_tonumber(L, 3)),
                static_cast<float>(::lua_tonumber(L, 4))
            };

            ImGui::ColorEdit3(label, col);

            ::lua_pushnumber(L, static_cast<double>(col[0]));
            ::lua_pushnumber(L, static_cast<double>(col[1]));
            ::lua_pushnumber(L, static_cast<double>(col[2]));
            return 3;
        }

        int lua_imgui_collapsing_header(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* label = ::lua_tolstring(L, 1, nullptr);
            if (!label) label = "Header";

            bool is_open = ImGui::CollapsingHeader(label);

            ::lua_pushboolean(L, is_open ? 1 : 0);
            return 1;
        }

        int lua_imgui_is_item_hovered(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            ::lua_pushboolean(L, ImGui::IsItemHovered() ? 1 : 0);
            return 1;
        }

        int lua_imgui_set_tooltip(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* text = ::lua_tolstring(L, 1, nullptr);
            if (text) {
                ImGui::SetTooltip("%s", text);
            }
            return 0;
        }
    }

    int load_imgui_module(lua::state s)
    {
        lua::stack_guard guard{ s };
        lua::create_table(guard);

        auto bind = [&](const char8_t* name, auto func) {
            lua::push(guard, name);
            lua::push(guard, func);
            lua::raw_set(guard, -3);
            };

        bind(u8"begin_window", lua_imgui_begin_window);
        bind(u8"end_window", lua_imgui_end_window);
        bind(u8"text", lua_imgui_text);
        bind(u8"button", lua_imgui_button);
        bind(u8"separator", lua_imgui_separator);
        bind(u8"same_line", lua_imgui_same_line);
        bind(u8"checkbox", lua_imgui_checkbox);
        bind(u8"slider_float", lua_imgui_slider_float);
        bind(u8"slider_int", lua_imgui_slider_int);
        bind(u8"color_edit3", lua_imgui_color_edit3);
        bind(u8"collapsing_header", lua_imgui_collapsing_header);
        bind(u8"is_item_hovered", lua_imgui_is_item_hovered);
        bind(u8"set_tooltip", lua_imgui_set_tooltip);

        return static_cast<int>(guard.release());
    }
}
