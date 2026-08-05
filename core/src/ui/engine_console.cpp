#include "ui/engine_console.hpp"
#include "command_manager.hpp"
#include "core.hpp"

#include <imgui.h>
#include <fstream>
#include <filesystem>

// We call ImGui's native handler safely inside our own render loop now!
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace windower::ui
{
    std::deque<std::u8string> engine_console::s_log_buffer = {
        u8"==================================================",
        u8" NextXI Advanced Engine Console (ImGui)",
        u8"==================================================",
        u8" * The console is fully active! Type your commands below.",
        u8" * Type 'help' and press Enter for a list of commands." };

    std::mutex g_console_mutex;

    void engine_console::toggle() noexcept
    {
        m_visible = !m_visible;
        m_scroll_to_bottom = true;
    }

    bool engine_console::is_visible() const noexcept { return m_visible; }

    std::optional<::LRESULT> engine_console::process_message(::MSG const& message) noexcept
    {
        if (message.message == WM_KEYDOWN || message.message == WM_SYSKEYDOWN)
        {
            if (message.wParam == VK_INSERT)
            {
                toggle();
                return 0;
            }
            if (message.wParam == VK_ESCAPE && m_visible)
            {
                m_visible = false;
                return 0;
            }
        }

        if (m_visible)
        {
            // Intercept mouse clicks, movement, and keyboard typing on the Main Thread
            // Lock them in the vault so they can be processed on the Render Thread!
            if ((message.message >= WM_KEYFIRST && message.message <= WM_KEYLAST) ||
                (message.message >= WM_MOUSEFIRST && message.message <= WM_MOUSELAST) ||
                message.message == WM_CHAR)
            {
                std::lock_guard<std::mutex> lock(m_msg_mutex);
                m_msg_queue.push_back(message);
                return 0; // Block FFXI from acting on these inputs
            }
        }

        return std::nullopt;
    }

    int engine_console::text_edit_callback_stub(ImGuiInputTextCallbackData* data)
    {
        auto* inst = static_cast<engine_console*>(data->UserData);
        return inst->text_edit_callback(data);
    }

    int engine_console::text_edit_callback(ImGuiInputTextCallbackData* data)
    {
        if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
        {
            const int prev_history_pos = m_history_index;
            if (data->EventKey == ImGuiKey_UpArrow)
            {
                if (m_history_index == -1)
                    m_history_index = m_history.size() - 1;
                else if (m_history_index > 0)
                    m_history_index--;
            }
            else if (data->EventKey == ImGuiKey_DownArrow)
            {
                if (m_history_index != -1)
                    if (++m_history_index >= (int)m_history.size())
                        m_history_index = -1;
            }

            if (prev_history_pos != m_history_index)
            {
                const char* history_str = (m_history_index >= 0) ? reinterpret_cast<const char*>(m_history[m_history_index].c_str()) : "";
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, history_str);
            }
        }
        return 0;
    }

    void engine_console::render(context& /*ctx*/) noexcept
    {
        if (!m_visible) return;

        // 1. UNLOCK THE VAULT SAFELY ON THE RENDER THREAD
        // Drain the message queue and feed it directly into ImGui
        {
            std::lock_guard<std::mutex> lock(m_msg_mutex);
            for (auto const& msg : m_msg_queue)
            {
                ImGui_ImplWin32_WndProcHandler(msg.hwnd, msg.message, msg.wParam, msg.lParam);
            }
            m_msg_queue.clear();
        }

        // 2. FORCE MOUSE CURSOR
        // FFXI hides the OS cursor. ImGui must draw its own so you can grab the window edges.
        ImGui::GetIO().MouseDrawCursor = true;

        ImGui::SetNextWindowSize(ImVec2(700, 450), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("NextXI Console", &m_visible, ImGuiWindowFlags_NoCollapse))
        {
            ImGui::End();
            return;
        }

        const float footer_height = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height), false, ImGuiWindowFlags_HorizontalScrollbar))
        {
            std::lock_guard<std::mutex> lock{ g_console_mutex };
            for (auto const& line : s_log_buffer)
            {
                ImGui::TextUnformatted(reinterpret_cast<char const*>(line.c_str()));
            }

            if (m_scroll_to_bottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            {
                ImGui::SetScrollHereY(1.0f);
                m_scroll_to_bottom = false;
            }
        }
        ImGui::EndChild();

        ImGui::Separator();

        bool reclaim_focus = false;
        ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory;

        // 3. AUTO-FOCUS TEXT BOX
        if (ImGui::IsWindowAppearing())
        {
            ImGui::SetKeyboardFocusHere();
        }

        ImGui::PushItemWidth(-1.0f);
        if (ImGui::InputText("##Input", m_input_buffer, IM_ARRAYSIZE(m_input_buffer), input_flags, &text_edit_callback_stub, this))
        {
            std::u8string cmd_str = reinterpret_cast<char8_t*>(m_input_buffer);
            m_input_buffer[0] = '\0';

            if (!cmd_str.empty())
            {
                if (m_history.empty() || m_history.back() != cmd_str)
                    m_history.push_back(cmd_str);
                m_history_index = -1;

                if (cmd_str == u8"clear")
                {
                    std::lock_guard<std::mutex> lock{ g_console_mutex };
                    s_log_buffer.clear();
                }
                else if (cmd_str == u8"help")
                {
                    push_log(u8"--- Console Commands ---");
                    push_log(u8" clear           : Erases all text in the console.");
                    push_log(u8" help            : Displays this command list.");
                    push_log(u8" /load <addon>   : Loads an addon.");
                    push_log(u8" /unload <addon> : Unloads an addon.");
                    push_log(u8"------------------------");
                }
                else
                {
                    core::instance().run_on_next_frame([cmd = cmd_str]() {
                        command_manager::instance().handle_command(cmd, command_source::console);
                        });
                }
                m_scroll_to_bottom = true;
            }
            reclaim_focus = true;
        }
        ImGui::PopItemWidth();

        ImGui::SetItemDefaultFocus();
        if (reclaim_focus) ImGui::SetKeyboardFocusHere(-1);

        ImGui::End();
    }

    void engine_console::push_log(std::u8string_view text) noexcept
    {
        std::lock_guard<std::mutex> lock{ g_console_mutex };

        auto process_and_push = [](std::u8string_view raw_line) {
            std::u8string line{ raw_line };
            if (!line.empty() && line.back() == u8'\r') line.pop_back();
            if (line.empty()) return;

            if (line.find(u8"packet_service") != std::u8string::npos) return;
            if (line.find(u8"linkshell_service") != std::u8string::npos) return;
            if (line.find(u8"windower::package") != std::u8string::npos) return;
            if (line.find(u8"PKG:P2") != std::u8string::npos) return;
            if (line.find(u8"PKG:F2") != std::u8string::npos) return;

            s_log_buffer.emplace_back(std::move(line));
            };

        std::u8string::size_type start = 0;
        std::u8string::size_type pos;
        while ((pos = text.find(u8'\n', start)) != std::u8string::npos)
        {
            process_and_push(text.substr(start, pos - start));
            start = pos + 1;
        }
        if (start < text.length())
        {
            process_and_push(text.substr(start));
        }

        while (s_log_buffer.size() > max_log_lines)
        {
            s_log_buffer.pop_front();
        }
    }
}
