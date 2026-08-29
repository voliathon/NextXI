#include "cloak.hpp"
#include "command_manager.hpp"
#include "core.hpp"
#include "crash_handler.hpp"
#include "library.hpp"
#include "unicode.hpp"
#include "utilities/logger.hpp"
#include "hooks/kernel32_internal.hpp"
#include "hooks/user32_window_find.hpp"
#include <windows.h>

namespace
{
    void enable_process_dpi_awareness()
    {
        // Rely purely on user32.dll to avoid loader lock access violations during suspended init
        ::SetProcessDPIAware();
    }
}

extern "C"
{
    static ::DWORD WINAPI initialize_thread(::LPVOID) noexcept(false)
    {
        windower::logger::sync_trace("initialize_thread: STARTED");
        windower::crash_handler::initialize();
        windower::logger::sync_trace("initialize_thread: crash_handler OK");
        windower::pin_and_cloak();
        windower::logger::sync_trace("initialize_thread: pin_and_cloak OK");
        enable_process_dpi_awareness();
        windower::logger::sync_trace("initialize_thread: dpi_awareness OK");
        windower::command_manager::initialize();
        windower::logger::sync_trace("initialize_thread: command_manager OK");
        windower::core::initialize();
        windower::logger::sync_trace("initialize_thread: core::initialize OK");
        return EXIT_SUCCESS;
    }

    ::BOOL WINAPI DllMain(::HINSTANCE hinstDLL, ::DWORD fdwReason, ::LPVOID)
    {
        switch (fdwReason)
        {
        default: break;
        case DLL_PROCESS_ATTACH:
            windower::logger::sync_trace_clear();
            windower::logger::sync_trace("DllMain: DLL_PROCESS_ATTACH Hit");

            // --- CRITICAL FIX: SYNCHRONOUS HOOKING ---
            // We must hook the Mutex and Window APIs on the main thread BEFORE DllMain returns.
            // If we wait for initialize_thread to do it, FFXI's main thread races ahead, 
            // sees the first instance, and calls ExitProcess(-1).
            windower::logger::sync_trace("DllMain: Installing anti-instance hooks synchronously...");
            windower::kernel32::install_sync();
            windower::user32::install_window_find();
            windower::kernel32::install_misc();
            windower::logger::sync_trace("DllMain: Anti-instance hooks installed.");

            return ::CreateThread(
                nullptr, 0, &::initialize_thread, hinstDLL, 0, nullptr)
                ? TRUE
                : FALSE;
        }
        return TRUE;
    }
}
