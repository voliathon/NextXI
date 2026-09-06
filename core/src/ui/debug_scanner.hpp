#pragma once
#include <string_view>
#include <functional>

namespace windower::ui::debug_scanner
{
    using logger_callback = std::function<void(std::u8string_view)>;

    // Main UI entry point
    void render_debug_tab(logger_callback const& log, bool& focus_console) noexcept;
}
