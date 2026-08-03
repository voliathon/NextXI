#ifndef WINDOWER_DEBUG_CONSOLE_HPP
#define WINDOWER_DEBUG_CONSOLE_HPP

#include <atomic>
#include <mutex>
#include <thread>

namespace windower
{

class debug_console
{
public:
    static void initialize(bool = false);
    static debug_console& instance() noexcept;

    debug_console(debug_console const&) = delete;
    debug_console(debug_console&&)      = delete;

    debug_console& operator=(debug_console const&) = delete;
    debug_console& operator=(debug_console&&) = delete;

    bool open() const noexcept;
    void open(bool);

private:
    std::atomic<bool> m_open = false;
    std::thread m_thread;
    std::mutex m_mutex;

    debug_console() = default;
    ~debug_console() noexcept;

    void run();
};

}

#endif