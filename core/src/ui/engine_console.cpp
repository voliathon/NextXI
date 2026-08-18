#include "ui/engine_console.hpp"
#include "ui/system_diagnostics.hpp"
#include "command_manager.hpp"
#include "core.hpp"
#include "addon/addon_manager.hpp"
#include "hooks/user32_internal.hpp"
#include "ui/debug_scanner.hpp" 
#include "addon/modules/player_scanner.hpp" 

#include <imgui.h>
#include <fstream>
#include <filesystem>
#include <gsl/gsl>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern "C" char const* get_ffxi_player_ffi();

namespace windower::ui
{
    std::deque<std::u8string> engine_console::s_log_buffer = {
        u8"==================================================",
        u8" NextXI Advanced Engine Console",
        u8"==================================================",
        u8" * The console is fully active! Type your commands below.",
        u8" * Type 'help' and press Enter for a list of commands." };

    std::mutex g_console_mutex;
    std::atomic<bool> engine_console::s_force_open{ false };
    bool g_focus_console_tab = false;

    void engine_console::toggle() noexcept
    {
        m_visible = !m_visible;
        m_scroll_to_bottom = true;
        if (m_visible) ::ClipCursor(nullptr);
    }

    bool engine_console::is_visible() const noexcept { return m_visible; }

    std::optional<::LRESULT> engine_console::process_message(::MSG const& message) noexcept
    {
        if (message.message == WM_KEYDOWN || message.message == WM_SYSKEYDOWN)
        {
            if (message.wParam == VK_INSERT) { toggle(); return 0; }
            if (message.wParam == VK_ESCAPE && m_visible) { m_visible = false; return 0; }
        }

        if (m_visible)
        {
            if ((message.message >= WM_KEYFIRST && message.message <= WM_KEYLAST) ||
                (message.message >= WM_MOUSEFIRST && message.message <= WM_MOUSELAST) ||
                message.message == WM_CHAR)
            {
                std::lock_guard<std::mutex> lock(m_msg_mutex);
                m_msg_queue.push_back(message);
                return 0;
            }
        }
        return std::nullopt;
    }

    int engine_console::text_edit_callback_stub(ImGuiInputTextCallbackData* data)
    {
        if (!data || !data->UserData) return 0;
        gsl::not_null<engine_console*> const inst = static_cast<engine_console*>(data->UserData);
        return inst->text_edit_callback(data);
    }

    int engine_console::text_edit_callback(ImGuiInputTextCallbackData* data)
    {
        if (!data) return 0;
        gsl::not_null<ImGuiInputTextCallbackData*> const safe_data = data;

        if (safe_data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
        {
            int const prev_history_pos = m_history_index;
            if (safe_data->EventKey == ImGuiKey_UpArrow)
            {
                if (m_history_index == -1) m_history_index = gsl::narrow_cast<int>(m_history.size()) - 1;
                else if (m_history_index > 0) m_history_index--;
            }
            else if (safe_data->EventKey == ImGuiKey_DownArrow)
            {
                if (m_history_index != -1)
                    if (++m_history_index >= gsl::narrow_cast<int>(m_history.size())) m_history_index = -1;
            }

            if (prev_history_pos != m_history_index)
            {
                const char* history_str = (m_history_index >= 0) ? reinterpret_cast<const char*>(m_history.at(m_history_index).c_str()) : "";
                safe_data->DeleteChars(0, safe_data->BufTextLen);
                safe_data->InsertChars(0, history_str);
            }
        }
        return 0;
    }

    bool engine_console::update_player_state() noexcept
    {
        return windower::player_scanner::get_local_player_json() != nullptr;
    }

    void engine_console::render_console_tab() noexcept
    {
        ImGuiTabItemFlags tab_flags = 0;
        if (g_focus_console_tab) { tab_flags |= ImGuiTabItemFlags_SetSelected; g_focus_console_tab = false; }

        if (ImGui::BeginTabItem("Console", nullptr, tab_flags))
        {
            const float footer_height = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

            if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height), false, ImGuiWindowFlags_HorizontalScrollbar))
            {
                std::lock_guard<std::mutex> lock{ g_console_mutex };
                for (auto const& line : s_log_buffer)
                    ImGui::TextUnformatted(reinterpret_cast<char const*>(line.c_str()));

                if (m_scroll_to_bottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                {
                    ImGui::SetScrollHereY(1.0f);
                    m_scroll_to_bottom = false;
                }
            }
            ImGui::EndChild();
            ImGui::Separator();

            bool reclaim_focus = false;
            constexpr ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory;

            if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();

            ImGui::PushItemWidth(-1.0f);
            if (ImGui::InputText("##Input", &m_input_buffer[0], std::size(m_input_buffer), input_flags, &text_edit_callback_stub, this))
            {
                std::u8string cmd_str = reinterpret_cast<char8_t*>(&m_input_buffer[0]);
                m_input_buffer[0] = '\0';

                if (!cmd_str.empty())
                {
                    if (m_history.empty() || m_history.back() != cmd_str) m_history.push_back(cmd_str);
                    m_history_index = -1;

                    if (cmd_str == u8"clear") {
                        std::lock_guard<std::mutex> lock{ g_console_mutex };
                        s_log_buffer.clear();
                    }
                    else if (cmd_str == u8"export") {
                        try {
                            auto export_dir = core::instance().settings.user_path / "NextXI_Logs";
                            std::filesystem::create_directories(export_dir);
                            auto export_path = export_dir / "console_export.txt";

                            bool success = false;
                            {
                                std::lock_guard<std::mutex> lock{ g_console_mutex };
                                std::ofstream out(export_path, std::ios::binary);
                                if (out) {
                                    for (auto const& line : s_log_buffer) {
                                        out.write(reinterpret_cast<char const*>(line.data()), line.size());
                                        out.write("\r\n", 2);
                                    }
                                    success = true;
                                }
                            }

                            if (success) push_log(u8"--- Exported to: " + export_path.u8string() + u8" ---");
                            else push_log(u8"--- ERROR: Failed to write export file ---");
                        }
                        catch (...) { push_log(u8"--- EXCEPTION: Failed to create export directory ---"); }
                    }
                    else if (cmd_str == u8"addons") {
                        std::lock_guard<std::mutex> lock{ g_console_mutex };
                        s_log_buffer.emplace_back(u8"--- Active Addons ---");
                        int count = 0;
                        if (core::instance().addon_manager) {
                            for (auto const& a : m_browser.get_cached_addons()) {
                                if (core::instance().addon_manager->get(a.name)) {
                                    s_log_buffer.emplace_back(u8" - " + a.name);
                                    count++;
                                }
                            }
                        }
                        if (count == 0) s_log_buffer.emplace_back(u8" None.");
                        s_log_buffer.emplace_back(u8"---------------------");
                    }
                    else if (cmd_str.find(u8"autoload") == 0) {
                        std::string args(cmd_str.begin() + 8, cmd_str.end());
                        args.erase(0, args.find_first_not_of(" \t"));
                        if (args.empty()) push_log(u8"Usage: //autoload <profile_name> (e.g. //autoload global)");
                        else {
                            push_log(u8"Auto-loading profile: " + std::u8string(args.begin(), args.end()));
                            addon_browser::run_autoload(args);
                        }
                    }
                    else if (cmd_str == u8"help") {
                        push_log(u8"--- Console Commands ---");
                        push_log(u8" clear               : Erases all text.");
                        push_log(u8" export              : Dumps history to console_export.txt.");
                        push_log(u8" addons              : Lists all active addons.");
                        push_log(u8" autoload <name>     : Loads a character profile.");
                        push_log(u8" //load <addon>      : Loads an addon.");
                        push_log(u8" //unload <addon>    : Unloads an addon.");
                    }
                    else {
                        std::u8string const echo_msg = u8"> Executing: " + cmd_str;
                        push_log(echo_msg);
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
            ImGui::EndTabItem();
        }
    }

    void engine_console::render(context& /*ctx*/) noexcept
    {
        if (!::GetModuleHandleW(L"FFXiMain.dll"))
        {
            m_visible = false;
            m_was_visible = false;
            s_force_open = false;
            return;
        }

        {
            std::lock_guard<std::mutex> lock(m_msg_mutex);
            for (auto const& msg : m_msg_queue)
                ImGui_ImplWin32_WndProcHandler(msg.hwnd, msg.message, msg.wParam, msg.lParam);
            m_msg_queue.clear();
        }

        if (m_was_visible && !m_visible) { ::ClipCursor(nullptr); m_was_visible = false; }
        else if (!m_was_visible && m_visible) { m_was_visible = true; }

        if (!m_visible)
        {
            m_browser.reset_scan();
            return;
        }

        bool const player_active = update_player_state();

        // ====================================================================
        // STATE TRANSITION TRACKER - AUTOMATIC ADDON UNLOADER
        // ====================================================================
        static bool s_was_player_active = false;

        if (s_was_player_active && !player_active) {
            push_log(u8"--- SESSION ENDED: Unloading all active addons ---");

            if (core::instance().addon_manager) {
                for (auto const& a : m_browser.get_cached_addons()) {
                    if (core::instance().addon_manager->get(a.name)) {
                        // Safely queue the unload command for the next frame
                        core::instance().run_on_next_frame([cmd = u8"//unload " + a.name]() {
                            command_manager::instance().handle_command(cmd, command_source::console);
                            });
                    }
                }
            }
        }
        s_was_player_active = player_active;
        // ====================================================================

        if (player_active) {
            try { m_browser.check_directory_changes(); }
            catch (...) {}
        }
        else {
            m_browser.reset_scan();
        }

        ImGuiStyle& style = ImGui::GetStyle();
        ImVec2 const old_frame_padding = style.FramePadding;
        float const old_window_border = style.WindowBorderSize;
        ImVec2 const old_window_padding = style.WindowPadding;

        style.FramePadding.y = 12.0f;
        style.WindowBorderSize = 2.0f;
        style.WindowPadding = ImVec2(12.0f, 12.0f);

        ImGui::SetNextWindowSize(ImVec2(750, 550), ImGuiCond_FirstUseEver);
        bool const is_open = ImGui::Begin("NextXI Control Center", &m_visible, ImGuiWindowFlags_NoCollapse);

        style.FramePadding = old_frame_padding;

        if (!is_open)
        {
            ImGui::End();
            style.WindowBorderSize = old_window_border;
            style.WindowPadding = old_window_padding;
            return;
        }

        if (ImGui::BeginTabBar("ConsoleTabs"))
        {
            system_diagnostics::render_about_tab();

            if (player_active) {
                render_console_tab();

                if (ImGui::BeginTabItem("Debug Tools", nullptr, 0)) {
                    ImGui::Spacing();
                    ImGui::TextWrapped("Diagnostic and execution tools for the NextXI engine.");
                    ImGui::Spacing();

                    if (ImGui::CollapsingHeader("Memory Scanning", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::Spacing();

                        ImGui::TextWrapped("1. Find Player Status Block");
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Scans memory for your exact name and dumps the surrounding HP/MP/Job structures.");
                        static char s_search_name[64] = "Please Select a Character";
                        ImGui::InputText("Character Name", s_search_name, sizeof(s_search_name));

                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
                        if (ImGui::Button("SCAN FOR PLAYER STATUS BLOCK", ImVec2(-1, 35)))
                        {
                            debug_scanner::execute_status_scan(s_search_name, [this](std::u8string_view msg) { push_log(msg); });
                            g_focus_console_tab = true;
                        }
                        ImGui::PopStyleColor(2);

                        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                        ImGui::TextWrapped("2. Find Global Entity Array");
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Extracts every active entity currently loaded in your zone.");

                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                        if (ImGui::Button("SCAN GLOBAL ENTITY ARRAY", ImVec2(-1, 35)))
                        {
                            debug_scanner::execute_entity_scan([this](std::u8string_view msg) { push_log(msg); });
                            g_focus_console_tab = true;
                        }
                        ImGui::PopStyleColor(2);
                    }

                    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                    if (ImGui::Button("RESET AUTHENTICATION SCANNER", ImVec2(-1, 35))) {
                        windower::player_scanner::reset_scan();
                        push_log(u8"--- MANUAL AUTHENTICATION RESET ---");
                    }

                    ImGui::EndTabItem();
                }

                try { m_browser.render_tabs(); }
                catch (...) {}
            }
            else {
                if (ImGui::BeginTabItem("Addons (LOCKED)", nullptr, ImGuiTabItemFlags_NoReorder)) {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Addon Manager is locked.");
                    ImGui::Text("NextXI requires a valid Character Name to load Addon configurations.");

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::Text("Authentication Status:");
                    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), windower::player_scanner::get_diagnostic_message());

                    ImGui::EndTabItem();
                }
            }

            ImGui::EndTabBar();
        }

        ImGui::End();

        style.WindowBorderSize = old_window_border;
        style.WindowPadding = old_window_padding;
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
        if (start < text.length()) process_and_push(text.substr(start));

        while (s_log_buffer.size() > max_log_lines) s_log_buffer.pop_front();
    }
}
