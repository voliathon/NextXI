#pragma once
#include <string_view>
#include <functional>

namespace windower::ui::debug_scanner
{
    // Callback type so the scanner can push text to the UI console
    using logger_callback = std::function<void(std::u8string_view)>;

    void execute_entity_scan(logger_callback const& log) noexcept;
    void execute_status_scan(const char* search_name, logger_callback const& log) noexcept;
}
