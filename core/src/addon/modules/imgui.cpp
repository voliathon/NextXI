#include "addon/modules/imgui.hpp"
#include "addon/modules/imgui_internal.hpp"
#include "addon/lua.hpp"

namespace windower
{
    int load_imgui_module(lua::state s)
    {
        lua::stack_guard guard{ s };

        // Create the global imgui table
        lua::create_table(guard);

        // Delegate the bindings to the sub-modules
        imgui_bindings::bind_window(guard);
        imgui_bindings::bind_widgets(guard);
        imgui_bindings::bind_layout(guard);
        imgui_bindings::bind_style(guard);

        return static_cast<int>(guard.release());
    }
}
