#include "cloak.hpp"
#include "command_manager.hpp"
#include "core.hpp"
#include "crash_handler.hpp"
#include "library.hpp"
#include "unicode.hpp"

#include <windows.h>

#include <shellscalingapi.h>

namespace
{
void enable_process_dpi_awareness()
{
    if (windower::library shcore{u8"shcore.dll"})
    {
        if (auto ptr = shcore.get_function(u8"SetProcessDpiAwareness"))
        {
            using SetProcessDpiAwareness =
                ::HRESULT(WINAPI*)(::PROCESS_DPI_AWARENESS);

            if (SUCCEEDED(reinterpret_cast<SetProcessDpiAwareness>(ptr)(
                    PROCESS_PER_MONITOR_DPI_AWARE)))
            {
                return;
            }
        }
    }
    ::SetProcessDPIAware();
}
}

extern "C"
{
    static ::DWORD WINAPI initialize_thread(::LPVOID) noexcept(false)
    {
        windower::crash_handler::initialize();
        windower::pin_and_cloak();

        enable_process_dpi_awareness();

        windower::command_manager::initialize();
        windower::core::initialize();

        return EXIT_SUCCESS;
    }

    ::BOOL WINAPI DllMain(::HINSTANCE hinstDLL, ::DWORD fdwReason, ::LPVOID)
    {
        switch (fdwReason)
        {
        default: break;
        case DLL_PROCESS_ATTACH:
            return ::CreateThread(
                       nullptr, 0, &::initialize_thread, hinstDLL, 0, nullptr)
                     ? TRUE
                     : FALSE;
        }
        return TRUE;
    }
}