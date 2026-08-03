#include "logger.hpp"

#include "core.hpp"
#include "addon/error.hpp"
#include "addon/errors/package_error.hpp"
#include "errors/windower_error.hpp"
#include "ui/engine_console.hpp"
#include "unicode.hpp"
#include "utilities/string_helpers.hpp" // For windower::to_u8string

#include <windows.h>
#include <gsl/gsl>

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

namespace
{
    struct log_message
    {
        std::u8string text;
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
                windower::core::instance().run_on_next_frame(
                    [text = std::move(msg.text)]() {
                        std::u8string::size_type start = 0;
                        std::u8string::size_type pos;

                        while ((pos = text.find(u8'\n', start)) != std::u8string::npos)
                        {
                            auto line = text.substr(start, pos - start);
                            while (!line.empty() && line.back() == u8'\r') line.pop_back();

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
                            result.append(frame.source.value.substr(1));
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

    void unwrap_exception(std::u8string& result, std::size_t level, std::exception const& ex)
    {
        std::string_view const type_name = typeid(ex).name();
        for (auto const c : type_name)
        {
            result.push_back(gsl::narrow_cast<char8_t>(c));
        }

        result.append(u8"\n  What: ");
        auto const* const what_str = ex.what();
        if (what_str && *what_str != '\0') { // Dereference instead of [0]
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
} // namespace

void windower::logger::queue_log(std::u8string text, bool is_error)
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

std::u8string windower::logger::get_error_message(std::exception const& exception)
{
    std::u8string result;
    unwrap_exception(result, 0, exception);
    return result;
}

std::u8string windower::logger::process_output(std::u8string_view component, std::u8string_view text)
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
