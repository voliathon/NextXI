#pragma once
#include "debug_scanner.hpp"

namespace windower::ui::debug_scanner
{
    void render_value_scanner_window(bool& show, logger_callback const& log, bool& focus_console) noexcept;
}
