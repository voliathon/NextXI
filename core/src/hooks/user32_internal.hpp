#pragma once

#include <windows.h>
#include <stdio.h> // Required for our logging macro!

// --- NEXTXI LOGGING MACRO ---
// In Debug builds, this compiles into our tracking radar.
// In Release builds, the compiler literally deletes this, costing zero performance.
#ifdef _DEBUG
#define NEXTXI_LOG(format, ...) \
    { \
        char buffer[512]; \
        sprintf_s(buffer, "[NextXI-Telemetry] " format "\n", __VA_ARGS__); \
        ::OutputDebugStringA(buffer); \
    }
#else
#define NEXTXI_LOG(format, ...) do {} while(0)
#endif
// ----------------------------

namespace windower::user32
{
    // Shared Audio Utility
    void set_process_muted(bool mute) noexcept;

    // Input Hooks
    void install_input();
    void uninstall_input() noexcept;

    // Window & Message Hooks
    void install_window();
    void uninstall_window() noexcept;
}
