#include "addon/modules/imgui_internal.hpp"
#include "addon/lua.hpp"
#include "addon/unsafe.hpp" 
#include <lua.hpp>          
#include <imgui.h>

namespace windower::imgui_bindings
{
    namespace
    {
        int lua_imgui_separator(lua::state s) { ImGui::Separator(); return 0; }
        int lua_imgui_same_line(lua::state s) { ImGui::SameLine(); return 0; }

        // NEW: Wire up the spacing function
        int lua_imgui_spacing(lua::state s) { ImGui::Spacing(); return 0; }

        int lua_imgui_collapsing_header(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* label = ::lua_tolstring(L, 1, nullptr);
            bool is_open = ImGui::CollapsingHeader(label ? label : "Header");
            ::lua_pushboolean(L, is_open ? 1 : 0);
            return 1;
        }

        int lua_imgui_set_tooltip(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* text = ::lua_tolstring(L, 1, nullptr);
            if (text) ImGui::SetTooltip("%s", text);
            return 0;
        }

        int lua_imgui_begin_tooltip(lua::state s) { ImGui::BeginTooltip(); return 0; }
        int lua_imgui_end_tooltip(lua::state s) { ImGui::EndTooltip(); return 0; }

        int lua_imgui_begin_tab_bar(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* str_id = ::lua_tolstring(L, 1, nullptr);
            ::lua_pushboolean(L, ImGui::BeginTabBar(str_id ? str_id : "##tabs"));
            return 1;
        }
        int lua_imgui_end_tab_bar(lua::state s) { ImGui::EndTabBar(); return 0; }
        int lua_imgui_begin_tab_item(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* label = ::lua_tolstring(L, 1, nullptr);
            ::lua_pushboolean(L, ImGui::BeginTabItem(label ? label : "Tab"));
            return 1;
        }
        int lua_imgui_end_tab_item(lua::state s) { ImGui::EndTabItem(); return 0; }

        int lua_imgui_begin_table(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* str_id = ::lua_tolstring(L, 1, nullptr);
            int columns = static_cast<int>(::lua_tointeger(L, 2));
            int flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
            ::lua_pushboolean(L, ImGui::BeginTable(str_id ? str_id : "##table", columns, flags));
            return 1;
        }
        int lua_imgui_end_table(lua::state s) { ImGui::EndTable(); return 0; }
        int lua_imgui_table_next_row(lua::state s) { ImGui::TableNextRow(); return 0; }
        int lua_imgui_table_next_column(lua::state s) { ImGui::TableNextColumn(); return 0; }
        int lua_imgui_table_setup_column(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* label = ::lua_tolstring(L, 1, nullptr);
            ImGui::TableSetupColumn(label ? label : "");
            return 0;
        }
        int lua_imgui_table_headers_row(lua::state s) { ImGui::TableHeadersRow(); return 0; }

        int lua_imgui_tree_node(lua::state s) {
            auto L = lua::unsafe::unwrap(s);
            const char* label = ::lua_tolstring(L, 1, nullptr);
            ::lua_pushboolean(L, ImGui::TreeNode(label ? label : "Node") ? 1 : 0);
            return 1;
        }
        int lua_imgui_tree_pop(lua::state s) { ImGui::TreePop(); return 0; }
    }

    void bind_layout(lua::stack_guard& guard)
    {
        auto bind = [&](const char8_t* name, auto func) {
            lua::push(guard, name);
            lua::push(guard, func);
            lua::raw_set(guard, -3);
            };

        bind(u8"separator", lua_imgui_separator);
        bind(u8"same_line", lua_imgui_same_line);
        bind(u8"spacing", lua_imgui_spacing);
        bind(u8"collapsing_header", lua_imgui_collapsing_header);
        bind(u8"set_tooltip", lua_imgui_set_tooltip);
        bind(u8"begin_tooltip", lua_imgui_begin_tooltip);
        bind(u8"end_tooltip", lua_imgui_end_tooltip);
        bind(u8"begin_tab_bar", lua_imgui_begin_tab_bar);
        bind(u8"end_tab_bar", lua_imgui_end_tab_bar);
        bind(u8"begin_tab_item", lua_imgui_begin_tab_item);
        bind(u8"end_tab_item", lua_imgui_end_tab_item);
        bind(u8"begin_table", lua_imgui_begin_table);
        bind(u8"end_table", lua_imgui_end_table);
        bind(u8"table_next_row", lua_imgui_table_next_row);
        bind(u8"table_next_column", lua_imgui_table_next_column);
        bind(u8"table_setup_column", lua_imgui_table_setup_column);
        bind(u8"table_headers_row", lua_imgui_table_headers_row);
        bind(u8"tree_node", lua_imgui_tree_node);
        bind(u8"tree_pop", lua_imgui_tree_pop);
    }
}
