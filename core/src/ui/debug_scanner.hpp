#pragma once
#include <string_view>
#include <functional>

namespace windower::ui::debug_scanner
{
    using logger_callback = std::function<void(std::u8string_view)>;

    void execute_entity_scan(logger_callback const& log) noexcept;
    void execute_status_scan(const char* search_name, logger_callback const& log) noexcept;

    // Renders the Debug Tools ImGui Tab
    void render_debug_tab(logger_callback const& log, bool& focus_console) noexcept;
}
