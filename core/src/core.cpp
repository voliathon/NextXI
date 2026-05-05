/*
 * Copyright © Windower Dev Team
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"),to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "core.hpp"

#include "addon/error.hpp"
#include "addon/errors/package_error.hpp"

#include "command_handlers.hpp"
#include "command_manager.hpp"
#include "crash_handler.hpp"
#include "debug_console.hpp"
#include "hooks/advapi32.hpp"
#include "hooks/d3d8.hpp"
#include "hooks/ddraw.hpp"
#include "hooks/dinput8.hpp"
#include "hooks/ffximain.hpp"
#include "hooks/imm32.hpp"
#include "hooks/kernel32.hpp"
#include "hooks/user32.hpp"
#include "settings.hpp"
#include "ui/user_interface.hpp"
#include "ui/engine_console.hpp"
#include "unicode.hpp"
#include "utility.hpp"
#include <float.h>
#include <gsl/gsl>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace
{
    struct log_message
    {
        std::u8string text; // <-- Kept as u8string to prevent truncation!
        bool is_error;
    };

    std::mutex log_mutex;
    std::condition_variable log_cv;
    std::vector<log_message> log_queue;
    bool log_shutdown = false;
    std::thread log_thread;

    void log_worker()
    {
        auto output_handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
        auto error_handle = ::GetStdHandle(STD_ERROR_HANDLE);
        std::vector<log_message> local_queue;

        while (true)
        {
            {
                std::unique_lock<std::mutex> lock{ log_mutex };
                log_cv.wait(
                    lock, [] { return !log_queue.empty() || log_shutdown; });

                if (log_shutdown && log_queue.empty())
                {
                    break;
                }
                std::swap(log_queue, local_queue);
            }

            for (auto& msg : local_queue)
            {
                // Convert to wstring ONLY for the external developer console window
                auto w_text = windower::to_wstring(msg.text);
                auto handle = msg.is_error ? error_handle : output_handle;

                ::CONSOLE_SCREEN_BUFFER_INFO buffer_info;
                bool const has_info =
                    ::GetConsoleScreenBufferInfo(handle, &buffer_info);

                if (has_info && buffer_info.dwCursorPosition.X != 0)
                {
                    w_text.insert(w_text.begin(), L'\n');
                }

                if (msg.is_error)
                {
                    ::SetConsoleTextAttribute(
                        handle, FOREGROUND_RED | FOREGROUND_INTENSITY);
                }

                ::DWORD written = 0;
                ::WriteConsoleW(
                    handle, w_text.data(), w_text.size(), &written, nullptr);

                if (msg.is_error)
                {
                    ::SetConsoleTextAttribute(
                        handle, has_info ? buffer_info.wAttributes
                        : (FOREGROUND_RED | FOREGROUND_GREEN |
                            FOREGROUND_BLUE));
                }

                ::OutputDebugStringW(w_text.c_str());

                // --- SEND TO IN-GAME UI CONSOLE ---
                windower::core::instance().run_on_next_frame(
                    [text = std::move(msg.text)]() {
                        // Split the text explicitly by \n so the UI perfectly renders every line!
                        std::u8string::size_type start = 0;
                        std::u8string::size_type pos;

                        while ((pos = text.find(u8'\n', start)) != std::u8string::npos)
                        {
                            auto line = text.substr(start, pos - start);
                            while (!line.empty() && line.back() == u8'\r') line.pop_back(); // Strip carriage returns

                            if (!line.empty()) {
                                windower::ui::engine_console::push_log(line);
                            }
                            start = pos + 1;
                        }

                        auto final_line = text.substr(start);
                        while (!final_line.empty() && final_line.back() == u8'\r') final_line.pop_back();

                        if (!final_line.empty()) {
                            windower::ui::engine_console::push_log(final_line);
                        }
                    });
            }
            local_queue.clear();
        }
    }

    struct async_logger_cleanup
    {
        constexpr async_logger_cleanup() noexcept = default;
        async_logger_cleanup(async_logger_cleanup const&) = delete;
        async_logger_cleanup(async_logger_cleanup&&) = delete;
        async_logger_cleanup& operator=(async_logger_cleanup const&) = delete;
        async_logger_cleanup& operator=(async_logger_cleanup&&) = delete;

        ~async_logger_cleanup()
        {
            {
                std::lock_guard<std::mutex> lock{ log_mutex };
                log_shutdown = true;
            }
            log_cv.notify_all();
            if (log_thread.joinable())
            {
                log_thread.detach();
            }
        }
    } cleanup_logger;

    void queue_log(std::u8string text, bool is_error)
    {
        {
            std::lock_guard<std::mutex> lock{ log_mutex };
            if (!log_thread.joinable())
            {
                log_thread = std::thread{ log_worker };
            }
            log_queue.push_back({ std::move(text), is_error });
        }
        log_cv.notify_one();
    }

    // 1. Explicit Lua Stack Trace Extractor
    void format_lua_error(std::u8string& result, windower::lua::error const& exception)
    {
        if (exception.has_stack_trace())
        {
            for (auto const& frame : exception.stack_trace())
            {
                result.append(1, u8'\n');
                if (!frame.type.empty())
                {
                    result.append(1, u8'(');
                    result.append(frame.type);
                    result.append(1, u8')');
                }
                result.append(frame.name.empty() ? frame.name : u8"<unknown>");
                if (!frame.source.value.empty())
                {
                    result.append(u8"\n  ");
                    if (frame.source.type == u8"string")
                    {
                        if (gsl::at(frame.source.value, 0) == u8'=')
                            result.append(frame.source.value.begin() + 1, frame.source.value.end());
                        else
                        {
                            result.append(u8"[string]");
                            result.append(frame.source.value);
                        }
                    }
                    else
                    {
                        if (frame.source.type != u8"file")
                        {
                            result.append(1, u8'[');
                            result.append(frame.source.type);
                            result.append(1, u8']');
                        }
                        result.append(frame.source.value);
                    }

                    if (frame.source.line != 0)
                    {
                        result.append(1, u8':');
                        result.append(windower::to_u8string(frame.source.line));
                    }
                }
            }
        }
    }

    // 2. Brute-Force Unwrapper (Bypasses Template Slicing!)
    void unwrap_exception(std::u8string& result, std::size_t level, std::exception const& ex)
    {
        std::string_view const type_name = typeid(ex).name();
        std::transform(type_name.begin(), type_name.end(), std::back_inserter(result), [](auto c) noexcept { return gsl::narrow_cast<char8_t>(c); });

        result.append(u8"\n  What: ");
        auto const what_str = ex.what();
        if (what_str && what_str[0] != '\0') {
            result.append(windower::to_u8string(what_str));
        }
        else {
            result.append(u8"<empty>");
        }

        if (auto windower_err = dynamic_cast<windower::windower_error const*>(&ex)) {
            result.append(u8"\n  Message: ");
            result.append(windower_err->message());
        }

        if (auto pkg_err = dynamic_cast<windower::package_error const*>(&ex)) {
            result.append(u8"\n  Packages: ");
            for (auto const& p : pkg_err->packages()) {
                result.append(p);
                result.append(1, u8' ');
            }
        }

        if (auto lua_err = dynamic_cast<windower::lua::error const*>(&ex)) {
            format_lua_error(result, *lua_err);
        }

        if (auto nested = dynamic_cast<std::nested_exception const*>(&ex)) {
            if (auto nested_ptr = nested->nested_ptr()) {
                result.append(u8"\n  --- Nested Exception [");
                result.append(windower::to_u8string(level + 1));
                result.append(u8"] ---\n  ");

                try {
                    std::rethrow_exception(nested_ptr);
                }
                catch (windower::lua::error const& e) { unwrap_exception(result, level + 1, e); }
                catch (windower::package_error const& e) { unwrap_exception(result, level + 1, e); }
                catch (windower::windower_error const& e) { unwrap_exception(result, level + 1, e); }
                catch (std::exception const& e) { unwrap_exception(result, level + 1, e); }
                catch (...) { result.append(u8"<Unknown Exception Type>"); }
            }
        }
    }

    // 3. The new core handler
    std::u8string get_error_message(std::exception const& exception)
    {
        std::u8string result;
        unwrap_exception(result, 0, exception);
        return result;
    }

    std::u8string process_output(std::u8string_view component, std::u8string_view text)
    {
        if (component.empty()) { component = u8"core"; }
        auto const count = std::count(text.begin(), text.end(), u8'\n') + 1;
        std::u8string temp;
        temp.reserve(temp.size() + (component.size() + 3) * count);
        temp.append(1, u8'[');
        temp.append(component);
        temp.append(1, u8']');
        temp.append(1, u8' ');
        std::u8string::size_type start = 0;
        std::u8string::size_type pos;
        while ((pos = text.find(u8'\n', start)) != std::u8string::npos)
        {
            temp.append(text, start, pos - start + 1);
            temp.append(1, u8'[');
            temp.append(component);
            temp.append(1, u8']');
            temp.append(1, u8' ');
            start = pos + 1;
        }
        temp.append(text, start, std::u8string::npos);
        return temp;
    }

} // <-- End of the anonymous namespace


void windower::core::initialize() noexcept { instance(); }

windower::core& windower::core::instance() noexcept
{
    static core instance;
    return instance;
}

windower::core::core() noexcept
{
    kernel32::install();
    user32::install();
    advapi32::install();
    imm32::install();
    d3d8::install();
    dinput8::install();
    ddraw::install();

    settings.load();
    crash_handler::instance().dump_path(settings.temp_path);

    run_on_next_frame([]() mutable {
        debug_console::initialize(core::instance().settings.debug);

        core::instance().package_manager =
            std::make_unique<windower::package_manager>();
        core::instance().package_manager->update_all();

        auto& cmd = command_manager::instance();

        cmd.register_command(
            command_manager::layer::core, u8"", u8"install",
            command_handlers::install);
        cmd.register_command(
            command_manager::layer::core, u8"", u8"uninstall",
            command_handlers::uninstall);
        cmd.register_command(
            command_manager::layer::core, u8"", u8"update",
            command_handlers::update);
        cmd.register_command(
            command_manager::layer::core, u8"", u8"updateall",
            command_handlers::updateall);

        cmd.register_command(
            command_manager::layer::core, u8"", u8"load",
            command_handlers::load);
        cmd.register_command(
            command_manager::layer::core, u8"", u8"unload",
            command_handlers::unload);
        cmd.register_command(
            command_manager::layer::core, u8"", u8"reload",
            command_handlers::reload);

        cmd.register_command(
            command_manager::layer::core, u8"", u8"unloadall",
            command_handlers::unloadall);
        cmd.register_command(
            command_manager::layer::core, u8"", u8"reloadall",
            command_handlers::reloadall);

        cmd.register_command(
            command_manager::layer::core, u8"", u8"alias",
            command_handlers::alias, true);
        cmd.register_command(
            command_manager::layer::core, u8"", u8"unalias",
            command_handlers::unalias);

        cmd.register_command(
            command_manager::layer::core, u8"", u8"bind",
            command_handlers::bind, true);
        cmd.register_command(
            command_manager::layer::core, u8"", u8"unbind",
            command_handlers::unbind, true);
        cmd.register_command(
            command_manager::layer::core, u8"", u8"listbinds",
            command_handlers::listbinds, true);

        cmd.register_command(
            command_manager::layer::core, u8"", u8"exec",
            command_handlers::exec);
        cmd.register_command(
            command_manager::layer::core, u8"", u8"eval",
            command_handlers::eval, true);

        cmd.register_command(
            command_manager::layer::core, u8"", u8"reset",
            command_handlers::reset);

        cmd.register_command(
            command_manager::layer::core, u8"", u8"pkg", command_handlers::pkg);

        cmd.register_command(
            command_manager::layer::core, u8"", u8"nextwindow",
            command_handlers::nextwindow);

        cmd.register_command(
            command_manager::layer::core, u8"", u8"prevwindow",
            command_handlers::prevwindow);


    });
}

void windower::core::output(
    std::u8string_view component, std::u8string_view text,
    command_source source)
{
    if (source < command_source::client)
    {
        // Keep it strictly u8string so it never drops characters
        auto u8_text = process_output(component, text);
        u8_text.append(1, u8'\n');
        queue_log(std::move(u8_text), false);
    }
    else
    {
        instance().run_on_next_frame([text = process_output(component, text)] {
            ffximain::add_to_chat(text);
            });
    }
}

void windower::core::error(
    std::u8string_view component, std::u8string_view text,
    command_source source)
{
    // FORCE ALL ERRORS TO THE UI CONSOLE IN RAW UTF-8
    auto u8_text = process_output(component, text);
    u8_text.append(1, u8'\n');
    queue_log(std::move(u8_text), true);
}

void windower::core::error(
    std::u8string_view component, std::exception const& exception,
    command_source source)
{
    error(component, get_error_message(exception), source);
}

void windower::core::error(
    std::u8string_view component, std::exception_ptr exception,
    command_source source)
{
    if (exception)
    {
        try
        {
            std::rethrow_exception(exception);
        }
        catch (std::exception const& e)
        {
            error(component, e, source);
        }
    }
    else
    {
        error(component, u8"unknown error", source);
    }
}

void windower::core::update() noexcept
{
    if (!m_updated)
    {
        m_updated = true;

        // ==========================================
        // THE FPU AIRLOCK (LUAJIT 64-BIT MATH GUARD)
        // ==========================================
        class FpuStateGuard
        {
            unsigned int original_state;

        public:
            FpuStateGuard(FpuStateGuard const&) = delete;
            FpuStateGuard(FpuStateGuard&&) = delete;
            FpuStateGuard& operator=(FpuStateGuard const&) = delete;
            FpuStateGuard& operator=(FpuStateGuard&&) = delete;

            FpuStateGuard() noexcept
            {
                // Save FFXI's 32-bit state and force CPU to 64-bit (_PC_53)
                _controlfp_s(&original_state, 0, 0);
                _controlfp_s(nullptr, _PC_53, _MCW_PC);
            }
            ~FpuStateGuard()
            {
                // The microsecond Lua is done, restore FFXI's 32-bit state
                _controlfp_s(nullptr, original_state, _MCW_PC);
            }
        };

        // Activate the airlock!
        FpuStateGuard fpu_guard;

        // EVERYTHING BELOW THIS LINE RUNS SAFELY IN 64-BIT MODE
        scheduler::next_frame();
        script_environment.run_until_idle();
        if (addon_manager)
        {
            addon_manager->run_until_idle();
        }

        //Create a local, un-shared queue
        std::queue<std::function<void()>> local_functions;

        //Lock the mutex exactly once and steal all pending functions
        {
            std::lock_guard<std::mutex> guard{ m_queued_functions_mutex };
            std::swap(m_queued_functions, local_functions);
        } // Mutex is instantly unlocked here!

        //Execute the stolen functions completely lock-free
        while (!local_functions.empty())
        {
            try
            {
                local_functions.front()();
            }
            catch (std::exception const& e) // <--- Catch the variable 'e'
            {
                // 2. Properly route the error with a component tag
                error(u8"Background Task", e, command_source::client);
            }
            local_functions.pop();
        }
    }
}

void windower::core::begin_frame() noexcept
{
    ui.begin_frame();
    m_updated = false;
}

void windower::core::end_frame() noexcept { ui.end_frame(); }

void windower::core::run_on_next_frame(std::function<void()> function)
{
    std::lock_guard<std::mutex> guard{m_queued_functions_mutex};
    m_queued_functions.emplace(std::move(function));
}

std::optional<::LRESULT>
windower::core::process_message(::MSG const& msg) noexcept
{
    if (auto const result = ui.process_message(msg))
    {
        return result;
    }
    if (auto const result = binding_manager.process_message(msg))
    {
        return result;
    }
    return std::nullopt;
}
