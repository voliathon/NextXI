#ifndef WINDOWER_ADDON_MODULES_IMGUI_INTERNAL_HPP
#define WINDOWER_ADDON_MODULES_IMGUI_INTERNAL_HPP

#include "addon/lua.hpp"

namespace windower::imgui_bindings
{
    void bind_window(lua::stack_guard& guard);
    void bind_widgets(lua::stack_guard& guard);
    void bind_layout(lua::stack_guard& guard);
    void bind_style(lua::stack_guard& guard);
}

#endif
