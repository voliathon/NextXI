#include "addon/modules/imgui.hpp"
#include "addon/lua.hpp"
#include <imgui.h>
#include <string>

namespace windower
{
    namespace
    {
        // Lua: imgui.begin_window("My Window")
        int lua_imgui_begin(lua::state s)
        {
            lua::stack_guard guard{ s };
            auto name = lua::get<std::u8string>(s, 1);
            bool open = true;

            ImGui::Begin(reinterpret_cast<const char*>(name.c_str()), &open);

            lua::push(guard, open);
            return static_cast<int>(guard.release());
        }

        // Lua: imgui.end_window()
        int lua_imgui_end(lua::state s)
        {
            lua::stack_guard guard{ s };
            ImGui::End();
            return static_cast<int>(guard.release());
        }

        // Lua: imgui.text("Hello World!")
        int lua_imgui_text(lua::state s)
        {
            lua::stack_guard guard{ s };
            auto text = lua::get<std::u8string>(s, 1);

            ImGui::TextUnformatted(reinterpret_cast<const char*>(text.c_str()));

            return static_cast<int>(guard.release());
        }

        // Lua: clicked = imgui.button("Click Me")
        int lua_imgui_button(lua::state s)
        {
            lua::stack_guard guard{ s };
            auto label = lua::get<std::u8string>(s, 1);

            bool clicked = ImGui::Button(reinterpret_cast<const char*>(label.c_str()));

            lua::push(guard, clicked);
            return static_cast<int>(guard.release());
        }
    }

    int load_imgui_module(lua::state s)
    {
        lua::stack_guard guard{ s };

        // Create the imgui table in Lua
        lua::create_table(guard);

        // Bind imgui.begin_window
        lua::push(guard, u8"begin_window");
        lua::push(guard, lua_imgui_begin);
        lua::raw_set(guard, -3);

        // Bind imgui.end_window
        lua::push(guard, u8"end_window");
        lua::push(guard, lua_imgui_end);
        lua::raw_set(guard, -3);

        // Bind imgui.text
        lua::push(guard, u8"text");
        lua::push(guard, lua_imgui_text);
        lua::raw_set(guard, -3);

        // Bind imgui.button
        lua::push(guard, u8"button");
        lua::push(guard, lua_imgui_button);
        lua::raw_set(guard, -3);

        // Return the table
        return static_cast<int>(guard.release());
    }
}
