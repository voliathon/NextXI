#include "addon/modules/imgui_internal.hpp"
#include "addon/lua.hpp"
#include "addon/unsafe.hpp" 
#include <lua.hpp>          
#include <imgui.h>
#include <string.h> 

namespace windower::imgui_bindings
{
    namespace
    {
        int lua_imgui_text(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* text = ::lua_tolstring(L, 1, nullptr);
            if (text) ImGui::TextUnformatted(text);
            return 0;
        }

        int lua_imgui_text_colored(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            float r = static_cast<float>(::lua_tonumber(L, 1));
            float g = static_cast<float>(::lua_tonumber(L, 2));
            float b = static_cast<float>(::lua_tonumber(L, 3));
            float a = static_cast<float>(::lua_tonumber(L, 4));
            const char* text = ::lua_tolstring(L, 5, nullptr);
            if (text) ImGui::TextColored(ImVec4(r, g, b, a), "%s", text);
            return 0;
        }

        int lua_imgui_button(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* label = ::lua_tolstring(L, 1, nullptr);
            bool clicked = ImGui::Button(label ? label : "Button");
            ::lua_pushboolean(L, clicked ? 1 : 0);
            return 1;
        }

        int lua_imgui_checkbox(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* label = ::lua_tolstring(L, 1, nullptr);
            bool checked = ::lua_toboolean(L, 2) != 0;
            ImGui::Checkbox(label ? label : "Checkbox", &checked);
            ::lua_pushboolean(L, checked ? 1 : 0);
            return 1;
        }

        int lua_imgui_slider_float(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* label = ::lua_tolstring(L, 1, nullptr);
            float v = static_cast<float>(::lua_tonumber(L, 2));
            float min = static_cast<float>(::lua_tonumber(L, 3));
            float max = static_cast<float>(::lua_tonumber(L, 4));
            ImGui::SliderFloat(label ? label : "Slider", &v, min, max);
            ::lua_pushnumber(L, static_cast<double>(v));
            return 1;
        }

        int lua_imgui_slider_int(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* label = ::lua_tolstring(L, 1, nullptr);
            int v = static_cast<int>(::lua_tointeger(L, 2));
            int min = static_cast<int>(::lua_tointeger(L, 3));
            int max = static_cast<int>(::lua_tointeger(L, 4));
            ImGui::SliderInt(label ? label : "Slider", &v, min, max);
            ::lua_pushinteger(L, v);
            return 1;
        }

        int lua_imgui_color_edit3(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* label = ::lua_tolstring(L, 1, nullptr);
            float col[3] = {
                static_cast<float>(::lua_tonumber(L, 2)),
                static_cast<float>(::lua_tonumber(L, 3)),
                static_cast<float>(::lua_tonumber(L, 4))
            };
            ImGui::ColorEdit3(label ? label : "Color", col);
            ::lua_pushnumber(L, static_cast<double>(col[0]));
            ::lua_pushnumber(L, static_cast<double>(col[1]));
            ::lua_pushnumber(L, static_cast<double>(col[2]));
            return 3;
        }

        int lua_imgui_progress_bar(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            float fraction = static_cast<float>(::lua_tonumber(L, 1));
            const char* overlay = ::lua_tolstring(L, 2, nullptr);
            ImGui::ProgressBar(fraction, ImVec2(-FLT_MIN, 0), overlay);
            return 0;
        }

        int lua_imgui_begin_combo(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* label = ::lua_tolstring(L, 1, nullptr);
            const char* preview = ::lua_tolstring(L, 2, nullptr);
            ::lua_pushboolean(L, ImGui::BeginCombo(label ? label : "Combo", preview ? preview : ""));
            return 1;
        }

        int lua_imgui_end_combo(lua::state s) { ImGui::EndCombo(); return 0; }

        int lua_imgui_selectable(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* label = ::lua_tolstring(L, 1, nullptr);
            bool selected = ::lua_toboolean(L, 2) != 0;
            ::lua_pushboolean(L, ImGui::Selectable(label ? label : "Item", selected));
            return 1;
        }

        int lua_imgui_input_text(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* label = ::lua_tolstring(L, 1, nullptr);
            const char* text = ::lua_tolstring(L, 2, nullptr);
            char buffer[256];
            strncpy_s(buffer, text ? text : "", _TRUNCATE);
            bool changed = ImGui::InputText(label ? label : "##input", buffer, sizeof(buffer));
            ::lua_pushboolean(L, changed ? 1 : 0);
            ::lua_pushstring(L, buffer);
            return 2;
        }

        int lua_imgui_is_item_hovered(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            ::lua_pushboolean(L, ImGui::IsItemHovered() ? 1 : 0);
            return 1;
        }
    }

    void bind_widgets(lua::stack_guard& guard)
    {
        auto bind = [&](const char8_t* name, auto func) {
            lua::push(guard, name);
            lua::push(guard, func);
            lua::raw_set(guard, -3);
            };

        bind(u8"text", lua_imgui_text);
        bind(u8"text_colored", lua_imgui_text_colored);
        bind(u8"button", lua_imgui_button);
        bind(u8"checkbox", lua_imgui_checkbox);
        bind(u8"slider_float", lua_imgui_slider_float);
        bind(u8"slider_int", lua_imgui_slider_int);
        bind(u8"color_edit3", lua_imgui_color_edit3);
        bind(u8"progress_bar", lua_imgui_progress_bar);
        bind(u8"begin_combo", lua_imgui_begin_combo);
        bind(u8"end_combo", lua_imgui_end_combo);
        bind(u8"selectable", lua_imgui_selectable);
        bind(u8"input_text", lua_imgui_input_text);
        bind(u8"is_item_hovered", lua_imgui_is_item_hovered);
    }
}
