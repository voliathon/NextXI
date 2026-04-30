#ifndef WINDOWER_UI_ENGINE_CONSOLE_HPP
#define WINDOWER_UI_ENGINE_CONSOLE_HPP

#include "ui/context.hpp"
#include "ui/widget/scroll_panel.hpp"
#include "ui/widget/window.hpp"

#include <windows.h>

#include <deque>
#include <optional>
#include <string>

namespace windower::ui
{
class engine_console
{
public:
    void toggle() noexcept;
    bool is_visible() const noexcept;

    std::optional<::LRESULT> process_message(::MSG const& message) noexcept;
    void render(context& ctx) noexcept;

    static void push_log(std::u8string_view text) noexcept;

private:
    bool m_visible = false;
    widget::window_state m_window_state;
    widget::scroll_panel_state m_scroll_state;

    std::u8string m_input_buffer;
    std::size_t m_cursor_position =
        0; // Tracks where to insert text or blink the cursor!

    std::deque<std::u8string> m_history;
    std::size_t m_history_index = 0;

    static std::deque<std::u8string> s_log_buffer;
    static constexpr std::size_t max_log_lines = 1000;
};
}

#endif