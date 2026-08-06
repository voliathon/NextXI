#ifndef WINDOWER_UI_ENGINE_CONSOLE_HPP
#define WINDOWER_UI_ENGINE_CONSOLE_HPP

#include "ui/context.hpp"
#include "ui/addon_browser.hpp"

#include <imgui.h>
#include <windows.h>
#include <deque>
#include <string>
#include <optional>
#include <vector>
#include <mutex>
#include <atomic>

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
        bool m_was_visible = false;

        char m_input_buffer[2048] = "";
        std::deque<std::u8string> m_history;
        int m_history_index = -1;
        bool m_scroll_to_bottom = false;

        std::mutex m_msg_mutex;
        std::vector<::MSG> m_msg_queue;

        // The Sub-Systems!
        addon_browser m_browser;

        static std::deque<std::u8string> s_log_buffer;
        static constexpr std::size_t max_log_lines = 1000;
        static std::atomic<bool> s_force_open;

        int text_edit_callback(ImGuiInputTextCallbackData* data);
        static int text_edit_callback_stub(ImGuiInputTextCallbackData* data);
    };
}
#endif
