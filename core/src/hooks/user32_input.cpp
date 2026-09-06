#include "user32_internal.hpp"

#include "hooklib/hook.hpp"
#include "utilities/module_info.hpp"

#include <windows.h>
#include <bit>
#include <utility>

namespace
{

    auto hhook_counter = ::LONG{};
    auto current_cursor = ::HCURSOR{};
    bool cursor_overridden = false;

    namespace hooks
    {
        windower::hooklib::hook<decltype(::GetCursor)> GetCursor;
        windower::hooklib::hook<decltype(::SetCursor)> SetCursor;
        windower::hooklib::hook<decltype(::SetWindowsHookExA)> SetWindowsHookExA;
        windower::hooklib::hook<decltype(::SetWindowsHookExW)> SetWindowsHookExW;
        windower::hooklib::hook<decltype(::UnhookWindowsHookEx)> UnhookWindowsHookEx;
    }

    namespace callbacks
    {
        ::HCURSOR WINAPI GetCursor() noexcept
        {
            if (windower::is_game_module(WINDOWER_RETURN_ADDRESS))
            {
                return current_cursor;
            }
            return hooks::GetCursor();
        }

        ::HCURSOR WINAPI SetCursor(::HCURSOR hCursor) noexcept
        {
            if (windower::is_game_module(WINDOWER_RETURN_ADDRESS))
            {
                if (!cursor_overridden)
                {
                    hooks::SetCursor(hCursor);
                }
                return std::exchange(current_cursor, hCursor);
            }
            cursor_overridden = hCursor != nullptr;
            return hooks::SetCursor(cursor_overridden ? hCursor : current_cursor);
        }

        ::HHOOK WINAPI SetWindowsHookExA(
            int idHook, ::HOOKPROC lpfn, ::HINSTANCE hMod, ::DWORD dwThreadId) noexcept
        {
            if ((idHook == WH_KEYBOARD_LL || idHook == WH_KEYBOARD) &&
                windower::is_game_module(WINDOWER_RETURN_ADDRESS))
            {
                return std::bit_cast<::HHOOK>(
                    std::bit_cast<std::intptr_t>(windower::windower_module()) +
                    ::InterlockedIncrement(&hhook_counter));
            }
            return hooks::SetWindowsHookExA(idHook, lpfn, hMod, dwThreadId);
        }

        ::HHOOK WINAPI SetWindowsHookExW(
            int idHook, ::HOOKPROC lpfn, ::HINSTANCE hMod, ::DWORD dwThreadId) noexcept
        {
            if ((idHook == WH_KEYBOARD_LL || idHook == WH_KEYBOARD) &&
                windower::is_game_module(WINDOWER_RETURN_ADDRESS))
            {
                return std::bit_cast<::HHOOK>(
                    std::bit_cast<std::intptr_t>(windower::windower_module()) +
                    ::InterlockedIncrement(&hhook_counter));
            }
            return hooks::SetWindowsHookExW(idHook, lpfn, hMod, dwThreadId);
        }

        ::BOOL WINAPI UnhookWindowsHookEx(::HHOOK hhk) noexcept
        {
            if (windower::is_windower_module(hhk))
            {
                return TRUE;
            }
            return hooks::UnhookWindowsHookEx(hhk);
        }
    }

} // namespace

void windower::user32::install_input()
{
    current_cursor = ::GetCursor();

    hooks::GetCursor =
        hooklib::make_hook(u8"user32.dll", u8"GetCursor", callbacks::GetCursor);
    hooks::SetCursor =
        hooklib::make_hook(u8"user32.dll", u8"SetCursor", callbacks::SetCursor);

    hooks::SetWindowsHookExA = hooklib::make_hook(
        u8"user32.dll", u8"SetWindowsHookExA", callbacks::SetWindowsHookExA);
    hooks::SetWindowsHookExW = hooklib::make_hook(
        u8"user32.dll", u8"SetWindowsHookExW", callbacks::SetWindowsHookExW);
    hooks::UnhookWindowsHookEx = hooklib::make_hook(
        u8"user32.dll", u8"UnhookWindowsHookEx", callbacks::UnhookWindowsHookEx);
}

void windower::user32::uninstall_input() noexcept
{
    hooks::GetCursor = {};
    hooks::SetCursor = {};
    hooks::SetWindowsHookExA = {};
    hooks::SetWindowsHookExW = {};
    hooks::UnhookWindowsHookEx = {};
}
