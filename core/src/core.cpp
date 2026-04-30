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
    std::wstring text;
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
    auto error_handle  = ::GetStdHandle(STD_ERROR_HANDLE);
    std::vector<log_message> local_queue;

    while (true)
    {
        {
            // Sleep the background thread until there is a log message to
            // process
            std::unique_lock<std::mutex> lock{log_mutex};
            log_cv.wait(
                lock, [] { return !log_queue.empty() || log_shutdown; });

            if (log_shutdown && log_queue.empty())
            {
                break;
            }

            // Steal the pending messages instantly to release the lock back to
            // the game thread
            std::swap(log_queue, local_queue);
        }

        // Process all console output safely in the background
        for (auto& msg : local_queue)
        {
            auto handle = msg.is_error ? error_handle : output_handle;

            ::CONSOLE_SCREEN_BUFFER_INFO buffer_info;
            bool const has_info =
                ::GetConsoleScreenBufferInfo(handle, &buffer_info);

            // If the console cursor isn't at the start of a line, inject a
            // newline
            if (has_info && buffer_info.dwCursorPosition.X != 0)
            {
                msg.text.insert(msg.text.begin(), L'\n');
            }

            if (msg.is_error)
            {
                ::SetConsoleTextAttribute(
                    handle, FOREGROUND_RED | FOREGROUND_INTENSITY);
            }

            ::DWORD written = 0;
            ::WriteConsoleW(
                handle, msg.text.data(), msg.text.size(), &written, nullptr);

            if (msg.is_error)
            {
                ::SetConsoleTextAttribute(
                    handle, has_info ? buffer_info.wAttributes
                                     : (FOREGROUND_RED | FOREGROUND_GREEN |
                                        FOREGROUND_BLUE));
            }

            ::OutputDebugStringW(msg.text.c_str());

// --- SEND TO IN-GAME UI CONSOLE ---
            auto u8_text = windower::to_u8string(msg.text);
            // Strip trailing newlines so we don't get double-spacing in the UI
            while (!u8_text.empty() &&
                   (u8_text.back() == u8'\n' || u8_text.back() == u8'\r'))
            {
                u8_text.pop_back();
            }

            // The background log thread MUST NOT directly modify the UI
            // buffer! Hand the text safely back to the main FFXI thread via the
            // mutex-locked queue.
            windower::core::instance().run_on_next_frame(
                [text = std::move(u8_text)]() {
                    windower::ui::engine_console::push_log(text);
                });
        }
        local_queue.clear();
    }
}

// Automatically cleans up the background thread when Windower unloads
struct async_logger_cleanup
{
    constexpr async_logger_cleanup() noexcept                    = default;
    async_logger_cleanup(async_logger_cleanup const&)            = delete;
    async_logger_cleanup(async_logger_cleanup&&)                 = delete;
    async_logger_cleanup& operator=(async_logger_cleanup const&) = delete;
    async_logger_cleanup& operator=(async_logger_cleanup&&)      = delete;

    ~async_logger_cleanup()
    {
        {
            std::lock_guard<std::mutex> lock{log_mutex};
            log_shutdown = true;
        }
        log_cv.notify_all();
        if (log_thread.joinable())
        {
            // telling the DLL to let go of the thread and allow the OS to kill
            // the process immediately without waiting. This is necessary to
            // avoid deadlocks on shutdown if the thread is currently blocked in
            // a Windows API call that doesn't respond to cancellation (e.g.
            // WriteConsoleW).
            log_thread.detach();
        }
    }
} cleanup_logger;

void queue_log(std::wstring text, bool is_error)
{
    {
        std::lock_guard<std::mutex> lock{log_mutex};
        // Lazy-initialize the thread so it doesn't spin up unless a log
        // actually happens
        if (!log_thread.joinable())
        {
            log_thread = std::thread{log_worker};
        }
        log_queue.push_back({std::move(text), is_error});
    }
    log_cv.notify_one();
}

template<typename E>
void get_type_name(std::u8string& result, E const& exception)
{
    std::string_view const type_name = typeid(exception).name();
    std::transform(
        type_name.begin(), type_name.end(), std::back_inserter(result),
        [](auto c) noexcept { return gsl::narrow_cast<char8_t>(c); });
}

void get_error_message(
    std::u8string& result, windower::lua::error const& exception)
{
    std::string_view const what = exception.what();
    result.reserve(result.size() + what.size());
    std::copy(what.begin(), what.end(), std::back_inserter(result));

    if (windower::core::instance().settings.verbose_logging &&
        exception.has_stack_trace())
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
                    {
                        result.append(
                            frame.source.value.begin() + 1,
                            frame.source.value.end());
                    }
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
            if (!frame.locals.empty())
            {
                result.append(u8"\n  locals:");
                for (auto const& l : frame.locals)
                {
                    result.append(u8"\n    ");
                    result.append(l.name);
                    result.append(u8" = [");
                    result.append(l.type);
                    result.append(u8"]");
                    result.append(l.value);
                }
            }
            if (!frame.upvalues.empty())
            {
                result.append(u8"\n  upvalues:");
                for (auto const& u : frame.upvalues)
                {
                    result.append(u8"\n    ");
                    result.append(u.name);
                    result.append(u8" = [");
                    result.append(u.type);
                    result.append(u8"]");
                    result.append(u.value);
                }
            }
        }
    }
}

template<typename E>
void get_error_message(std::u8string& result, E const& exception)
{
    if (auto ptr = dynamic_cast<windower::lua::error const*>(&exception))
    {
        get_error_message(result, *ptr);
    }
    else
    {
        get_type_name(result, exception);
        result.append(u8"\n");
        auto const what = windower::to_u8string(exception.what());
        result.append(what);
        if (auto windower_error =
                dynamic_cast<windower::windower_error const*>(&exception))
        {
            result.append(u8": ");
            result.append(windower_error->message());
        }
    }
}

template<typename E>
void get_error_message(
    std::u8string& result, std::size_t level, E const& exception)
{
    get_error_message(result, exception);
    if (auto nested = dynamic_cast<std::nested_exception const*>(&exception))
    {
        if (auto nested_ptr = nested->nested_ptr())
        {
            try
            {
                std::rethrow_exception(nested_ptr);
            }
            catch (std::exception const& e)
            {
                result.append(u8"\nnested exception [");
                result.append(windower::to_u8string(level + 1));
                result.append(u8"]: \n");
                get_error_message(result, level + 1, e);
            }
        }
    }
}

template<typename E>
std::u8string get_error_message(E const& exception)
{
    std::u8string result;
    get_error_message(result, 0, exception);
    return result;
}

std::u8string
process_output(std::u8string_view component, std::u8string_view text)
{
    if (component.empty())
    {
        component = u8"core";
    }
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

}

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
        auto w_text =
            to_wstring(process_output(component, text)).append(1, L'\n');
        queue_log(std::move(w_text), false);
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
    if (source < command_source::client)
    {
        auto w_text =
            to_wstring(process_output(component, text)).append(1, L'\n');
        queue_log(std::move(w_text), true);
    }
    else
    {
        instance().run_on_next_frame([text = process_output(component, text)] {
            ffximain::add_to_chat(text, 0xA7);
        });
    }
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
        // THE BULLETPROOF AUTOLOADER (WINDOWER 5 NATIVE)
        // Waits until the engine is fully alive and the addon_manager exists
        // ==========================================
        static bool auto_loaded = false;
        static int boot_delay   = 0;

        if (!auto_loaded && addon_manager)
        {
            // Give the engine time to breathe (approx 2 seconds)
            if (boot_delay < 120)
            {
                boot_delay++;
            }
            else
            {
                try
                {
                    auto& cmd = command_manager::instance();

                    // 1. The Core Network & Data Providers
                    cmd.handle_command(
                        u8"/load packet_service",
                        windower::command_source::client);
                    cmd.handle_command(
                        u8"/load settings_service",
                        windower::command_source::client);
                    cmd.handle_command(
                        u8"/load resources_service",
                        windower::command_source::client);
                    cmd.handle_command(
                        u8"/load client_data_service",
                        windower::command_source::client);

                    // 2. The Dynamic Game-State Hosts
                    cmd.handle_command(
                        u8"/load account_service",
                        windower::command_source::client);
                    cmd.handle_command(
                        u8"/load player_service",
                        windower::command_source::client);
                    cmd.handle_command(
                        u8"/load target_service",
                        windower::command_source::client);
                    cmd.handle_command(
                        u8"/load world_service",
                        windower::command_source::client);
                    cmd.handle_command(
                        u8"/load items_service",
                        windower::command_source::client);
                    cmd.handle_command(
                        u8"/load key_items_service",
                        windower::command_source::client);
                    cmd.handle_command(
                        u8"/load action_service",
                        windower::command_source::client);
                    cmd.handle_command(
                        u8"/load automaton_service",
                        windower::command_source::client);
                    cmd.handle_command(
                        u8"/load linkshell_service",
                        windower::command_source::client);
                    cmd.handle_command(
                        u8"/load status_effects_service",
                        windower::command_source::client);
                    cmd.handle_command(
                        u8"/load treasure_service",
                        windower::command_source::client);

                    // 3. The Master Interface
                    cmd.handle_command(
                        u8"/load AddonManager",
                        windower::command_source::client);

                    // ONLY flag as successfully loaded if every command above
                    // fires without throwing an error!
                    auto_loaded = true;
                }
                catch (...)
                {
                    // The engine parser threw an error because it isn't awake
                    // yet. Knock the delay back down so it waits 1 more second,
                    // then tries the loop again.
                    boot_delay = 60;
                }
            }
        }
        // ==========================================

        // ==========================================
        // THE FPU AIRLOCK (LUAJIT 64-BIT MATH GUARD)
        // ==========================================
        class FpuStateGuard
        {
            unsigned int original_state;

        public:
            FpuStateGuard(FpuStateGuard const&)            = delete;
            FpuStateGuard(FpuStateGuard&&)                 = delete;
            FpuStateGuard& operator=(FpuStateGuard const&) = delete;
            FpuStateGuard& operator=(FpuStateGuard&&)      = delete;

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

        // Activate the airlock! It will automatically lock the CPU into 64-bit
        // mode here, and unlock it when it hits the closing bracket of the
        // update() loop.
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
            std::lock_guard<std::mutex> guard{m_queued_functions_mutex};
            std::swap(m_queued_functions, local_functions);
        } // Mutex is instantly unlocked here!

        //Execute the stolen functions completely lock-free
        while (!local_functions.empty())
        {
            try
            {
                local_functions.front()();
            }
            catch (std::exception const&)
            {
                error(u8"");
            }
            local_functions.pop();
        }
    } // <-- FpuStateGuard is automatically destroyed here, returning CPU to
      // 32-bit for FFXI!
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