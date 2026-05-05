#include "ui/engine_console.hpp"

#include "command_manager.hpp"
#include "core.hpp"
#include "ui/primitives.hpp"
#include "ui/widget/label.hpp"

#include <gsl/gsl>

#include <algorithm>
#include <fstream> // Required for writing the export file
#include <filesystem>
#include <mutex>

namespace windower::ui
{
std::deque<std::u8string> engine_console::s_log_buffer = {
    u8"==================================================",
    u8" Windower 5 Advanced Engine Console",
    u8"==================================================",
    u8" * The console is fully active! Type your commands below.",
    u8" * Use PageUp / PageDown to scroll history.",
    u8" * Use Up / Down arrows to recall previous commands.",
    u8" * Type 'help' and press Enter for a list of commands."};
std::mutex g_console_mutex;
bool g_auto_scroll = false;

void engine_console::toggle() noexcept { m_visible = !m_visible; }

bool engine_console::is_visible() const noexcept { return m_visible; }

std::optional<::LRESULT>
engine_console::process_message(::MSG const& message) noexcept
{
    if (message.message == WM_KEYDOWN || message.message == WM_SYSKEYDOWN)
    {
        // Insert toggles the console open and closed
        if (message.wParam == VK_INSERT)
        {
            toggle();
            return 0;
        }
        // Escape ONLY closes the console, never opens it
        if (message.wParam == VK_ESCAPE && m_visible)
        {
            m_visible = false;
            return 0;
        }
    }

    if (!m_visible)
        return std::nullopt;

    if (message.message >= WM_KEYFIRST && message.message <= WM_KEYLAST)
    {
        if (message.message == WM_CHAR)
        {
            char const c = gsl::narrow_cast<char>(message.wParam);
            if (c == '\b') // Backspace
            {
                if (m_cursor_position > 0)
                {
                    do
                    {
                        m_cursor_position--;
                        auto last = m_input_buffer[m_cursor_position];
                        m_input_buffer.erase(m_cursor_position, 1);
                        if ((last & 0xC0) != 0x80)
                            break; // UTF-8 safety check
                    }
                    while (m_cursor_position > 0);
                }
            }
            else if (c >= 32 && c <= 126) // Printable Characters
            {
                m_input_buffer.insert(m_cursor_position, 1, c);
                m_cursor_position++;
            }
        }
        else if (message.message == WM_KEYDOWN)
        {
            if (message.wParam == VK_LEFT) // Move Cursor Left
            {
                if (m_cursor_position > 0)
                {
                    do
                    {
                        m_cursor_position--;
                    }
                    while (m_cursor_position > 0 &&
                           (m_input_buffer[m_cursor_position] & 0xC0) == 0x80);
                }
            }
            else if (message.wParam == VK_RIGHT) // Move Cursor Right
            {
                if (m_cursor_position < m_input_buffer.size())
                {
                    do
                    {
                        m_cursor_position++;
                    }
                    while (m_cursor_position < m_input_buffer.size() &&
                           (m_input_buffer[m_cursor_position] & 0xC0) == 0x80);
                }
            }
            else if (message.wParam == VK_HOME) // Jump to start
            {
                m_cursor_position = 0;
            }
            else if (message.wParam == VK_END) // Jump to end
            {
                m_cursor_position = m_input_buffer.size();
            }
            else if (message.wParam == VK_DELETE) // Delete Key
            {
                if (m_cursor_position < m_input_buffer.size())
                {
                    std::size_t end_pos = m_cursor_position;
                    do
                    {
                        end_pos++;
                    }
                    while (end_pos < m_input_buffer.size() &&
                           (m_input_buffer[end_pos] & 0xC0) == 0x80);
                    m_input_buffer.erase(
                        m_cursor_position, end_pos - m_cursor_position);
                }
            }
            else if (message.wParam == VK_UP)
            {
                if (!m_history.empty() && m_history_index > 0)
                {
                    m_history_index--;
                    m_input_buffer    = m_history[m_history_index];
                    m_cursor_position = m_input_buffer.size();
                }
            }
            else if (message.wParam == VK_DOWN)
            {
                if (m_history_index + 1 < m_history.size())
                {
                    m_history_index++;
                    m_input_buffer    = m_history[m_history_index];
                    m_cursor_position = m_input_buffer.size();
                }
                else if (m_history_index + 1 == m_history.size())
                {
                    m_history_index++;
                    m_input_buffer.clear();
                    m_cursor_position = 0;
                }
            }
            else if (message.wParam == VK_PRIOR) // PageUp
            {
                m_scroll_state.offset.y =
                    std::max(0.f, m_scroll_state.offset.y - 100.f);
                g_auto_scroll = false;
            }
            else if (message.wParam == VK_NEXT) // PageDown
            {
                m_scroll_state.offset.y += 100.f;
            }
            else if (message.wParam == VK_RETURN) // Enter Key
            {
                if (!m_input_buffer.empty())
                {
                    if (m_history.empty() || m_history.back() != m_input_buffer)
                    {
                        m_history.push_back(m_input_buffer);
                        if (m_history.size() > 5)
                            m_history.pop_front();
                    }
                    m_history_index = m_history.size();

                    // --- NATIVE CONSOLE COMMANDS ---
                    if (m_input_buffer == u8"clear")
                    {
                        std::lock_guard<std::mutex> lock{g_console_mutex};
                        s_log_buffer.clear();
                    }
                    else if (m_input_buffer == u8"export")
                    {
                        bool success = false;

                        // Force the path to the Windows Temp folder so it is
                        // always easy to find
                        auto export_path =
                            core::instance().settings.user_path.parent_path() /
                            "console_export.log";

                        {
                            std::lock_guard<std::mutex> lock{g_console_mutex};

                            // Ensure the Windower temp directory actually
                            // exists
                            std::error_code ec;
                            std::filesystem::create_directories(
                                export_path.parent_path(), ec);

                            std::ofstream out(export_path, std::ios::binary);
                            if (out)
                            {
                                for (auto const& line : s_log_buffer)
                                {
                                    out.write(
                                        reinterpret_cast<char const*>(
                                            line.data()),
                                        line.size());
                                    out.write("\r\n", 2);
                                }
                                success = true;
                            }
                        }

                        if (success)
                        {
                            // Dynamically print the exact path to the console
                            // so you can see where it went
                            std::u8string success_msg = u8"--- Exported to: " +
                                                        export_path.u8string() +
                                                        u8" ---";
                            push_log(success_msg);
                        }
                        else
                        {
                            push_log(
                                u8"--- ERROR: Failed to write export file ---");
                        }
                    }
                    else if (m_input_buffer == u8"help")
                    {
                        push_log(u8"");
                        push_log(u8"--- Console Commands ---");
                        push_log(
                            u8" clear           : Erases all text in the "
                            u8"console.");
                        push_log(
                            u8" export          : Dumps the console history to "
                            u8"a text file.");
                        push_log(
                            u8" help            : Displays this command list.");
                        push_log(
                            u8" /pkg reload     : Rescans and reloads the "
                            u8"package manifest.");
                        push_log(
                            u8" /load <addon>   : Loads a specific addon into "
                            u8"memory.");
                        push_log(
                            u8" /unload <addon> : Unloads an active addon from "
                            u8"memory.");
                        push_log(
                            u8" /reload <addon> : Restarts an active addon.");
                        push_log(
                            u8" <cmd>           : Any other command is sent "
                            u8"directly to FFXI.");
                        push_log(u8"------------------------");
                        push_log(u8"");
                    }
                    else
                    {
                        core::instance().run_on_next_frame(
                            [cmd_str = m_input_buffer]() {
                                command_manager::instance().handle_command(
                                    cmd_str, command_source::console);
                            });
                    }

                    m_input_buffer.clear();
                    m_cursor_position = 0; // Reset cursor to 0 on enter
                    g_auto_scroll     = true;
                }
            }
        }
        return 0; // Aggressively block input from reaching FFXI
    }

    return std::nullopt;
}

void engine_console::render(context& ctx) noexcept
{
    if (!m_visible)
        return;

    auto const screen_width  = ctx.screen_size().width;
    auto const screen_height = ctx.screen_size().height;

    if (screen_width <= 0.f || screen_height <= 0.f)
        return;

    constexpr float input_height = 24.f;
    float const console_height   = screen_height * 0.4f;

    auto const scroll_id = make_id(this, 1);

    m_window_state.layer(layer::screen);
    m_window_state.style(widget::window_style::chromeless);
    m_window_state.color(windower::ui::color{0, 0, 0, 210});
    m_window_state.bounds({0.f, 0.f, screen_width, console_height});

    if (widget::begin_window(ctx, m_window_state))
    {
        // --- 1. RENDER SCROLL PANEL ---
        ctx.bounds({0.f, 0.f, screen_width, console_height - input_height});

        constexpr float line_height = 16.f;
        float const content_height  = s_log_buffer.size() * line_height;

        m_scroll_state.canvas_size = {screen_width, content_height};
        m_scroll_state.visibility_vertical =
            widget::scroll_bar_visibility::automatic;
        m_scroll_state.visibility_horizontal =
            widget::scroll_bar_visibility::hidden;

        if (g_auto_scroll)
        {
            m_scroll_state.offset.y = content_height;
            g_auto_scroll           = false;
        }

        widget::begin_scroll_panel(ctx, scroll_id, m_scroll_state);
        {
            std::lock_guard<std::mutex> lock{g_console_mutex};
            float current_y         = 0.f;
            float const label_width = std::max(0.f, screen_width - 16.f);

            for (auto const& line : s_log_buffer)
            {
                ctx.bounds({4.f, current_y, label_width, line_height});
                widget::label(ctx, line);
                current_y += line_height;
            }
        }
        widget::end_scroll_panel(ctx);

        // --- 2. RENDER CUSTOM INPUT LINE ---
        auto const input_bounds = rectangle{
            0.f, console_height - input_height, screen_width, console_height};

        primitive::set_texture(ctx, u8":system");
        primitive::rectangle(
            ctx, input_bounds, windower::ui::color{0, 0, 0, 255});

        // Split the string based on the cursor position to insert the blinking
        // caret
        bool const blink = (::GetTickCount64() / 500) % 2 == 0;
        std::u8string const display_text =
            u8"> " + m_input_buffer.substr(0, m_cursor_position) +
            (blink ? u8"|" : u8"") + m_input_buffer.substr(m_cursor_position);

        ctx.bounds(
            {4.f, console_height - input_height + 4.f, screen_width - 160.f,
             console_height});
        widget::label(ctx, display_text);

        ctx.bounds(
            {screen_width - 150.f, console_height - input_height + 4.f,
             screen_width, console_height});
        widget::label(ctx, u8"[PgUp/PgDn to Scroll]");
    }

    widget::end_window(ctx);
}

void engine_console::push_log(std::u8string_view text) noexcept
{
    std::lock_guard<std::mutex> lock{g_console_mutex};

    auto process_and_push = [](std::u8string_view raw_line) {
        std::u8string line{raw_line};
        if (!line.empty() && line.back() == u8'\r')
            line.pop_back();
        if (line.empty())
            return;

        // 1. Filter standard background services
        if (line.find(u8"packet_service") != std::u8string::npos)
            return;
        if (line.find(u8"linkshell_service") != std::u8string::npos)
            return;

        // 2. Filter early IPC network errors and package warnings
        if (line.find(u8"windower::package") != std::u8string::npos)
            return;
        if (line.find(u8"PKG:P2") != std::u8string::npos)
            return;
        if (line.find(u8"PKG:F2") != std::u8string::npos)
            return;
        if (line.find(u8"channel 'packets' not found") != std::u8string::npos)
            return;

        // 3. Filter Lua Stack Trace Dumps
        if (line.find(u8"(global)<unknown>") != std::u8string::npos)
            return;
        if (line.find(u8"(method)<unknown>") != std::u8string::npos)
            return;
        if (line.find(u8"(metamethod)<unknown>") != std::u8string::npos)
            return;
        if (line.find(u8"[core]   ") != std::u8string::npos)
            return; // Swallows indented Lua variables
        if (line.find(u8"[core] \t") != std::u8string::npos)
            return; // Swallows tabbed Lua variables

        // 4. Handle Core & Addon Loading Labels
        if (line.find(u8"[core]") != std::u8string::npos &&
            line.find(u8"loaded") != std::u8string::npos)
        {
            if (line.find(u8"_service") != std::u8string::npos ||
                line.find(u8"_data") != std::u8string::npos ||
                line.find(u8"AddonManager") != std::u8string::npos ||
                line.find(u8"NextXISDK") != std::u8string::npos ||
                line.find(u8"mime") != std::u8string::npos ||
                line.find(u8"socket") != std::u8string::npos)
            {
                return;
            }

            auto const pos = line.find(u8"[core]");
            if (pos != std::u8string::npos)
            {
                line.replace(pos, 6, u8"[addons]");
            }
        }

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

    g_auto_scroll = true;
}
}