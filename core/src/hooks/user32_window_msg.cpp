#include "user32_window_split.hpp"
#include "hooklib/hook.hpp"
#include "core.hpp"
//peek/dispatch loop injection

namespace
{
    namespace hooks
    {
        windower::hooklib::hook<decltype(::PeekMessageA)> PeekMessageA;
        windower::hooklib::hook<decltype(::DispatchMessageA)> DispatchMessageA;
    }

    namespace callbacks
    {
        ::BOOL WINAPI PeekMessageA(::LPMSG lpMsg, ::HWND hWnd, ::UINT wMsgFilterMin, ::UINT wMsgFilterMax, ::UINT wRemoveMsg) noexcept
        {
            auto& core = windower::core::instance();
            auto msg = ::MSG{};
            auto const flags = (wRemoveMsg & ~PM_REMOVE) | PM_NOREMOVE;
            while (::PeekMessageW(&msg, hWnd, wMsgFilterMin, wMsgFilterMax, flags))
            {
                if (core.process_message(msg))
                {
                    ::PeekMessageW(&msg, hWnd, wMsgFilterMin, wMsgFilterMax, PM_REMOVE | PM_NOYIELD);
                    ::TranslateMessage(&msg);
                }
                else
                {
                    return hooks::PeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg | PM_NOYIELD);
                }
            }
            return FALSE;
        }

        ::LRESULT WINAPI DispatchMessageA(const ::MSG* lpMsg) noexcept
        {
            if (windower::user32::check_class(lpMsg->hwnd, windower::user32::ffxi_class_atom))
            {
                if (auto const data = windower::user32::get_window_data(lpMsg->hwnd)) data->current = *lpMsg;
            }
            return hooks::DispatchMessageA(lpMsg);
        }
    }
}

void windower::user32::install_window_msg()
{
    hooks::PeekMessageA = hooklib::make_hook(u8"user32.dll", u8"PeekMessageA", callbacks::PeekMessageA);
    hooks::DispatchMessageA = hooklib::make_hook(u8"user32.dll", u8"DispatchMessageA", callbacks::DispatchMessageA);
}

void windower::user32::uninstall_window_msg() noexcept
{
    hooks::PeekMessageA = {}; hooks::DispatchMessageA = {};
}
