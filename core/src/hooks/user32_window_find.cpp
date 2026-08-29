#include "user32_window_find.hpp"
#include "user32_internal.hpp"
#include "hooklib/hook.hpp"
#include <windows.h>
#include <cstring>
#include <cwchar>

namespace
{
    bool is_game_window(::LPCSTR lpClassName, ::LPCSTR lpWindowName) noexcept
    {
        if (lpClassName && !IS_INTRESOURCE(lpClassName))
        {
            if (std::strstr(lpClassName, "FFXi") || std::strstr(lpClassName, "PlayOnline") || std::strstr(lpClassName, "POL")) return true;
        }
        if (lpWindowName && !IS_INTRESOURCE(lpWindowName))
        {
            if (std::strstr(lpWindowName, "FINAL FANTASY") || std::strstr(lpWindowName, "PlayOnline") || std::strstr(lpWindowName, "POL")) return true;
        }
        return false;
    }

    bool is_game_windowW(::LPCWSTR lpClassName, ::LPCWSTR lpWindowName) noexcept
    {
        if (lpClassName && !IS_INTRESOURCE(lpClassName))
        {
            if (std::wcsstr(lpClassName, L"FFXi") || std::wcsstr(lpClassName, L"PlayOnline") || std::wcsstr(lpClassName, L"POL")) return true;
        }
        if (lpWindowName && !IS_INTRESOURCE(lpWindowName))
        {
            if (std::wcsstr(lpWindowName, L"FINAL FANTASY") || std::wcsstr(lpWindowName, L"PlayOnline") || std::wcsstr(lpWindowName, L"POL")) return true;
        }
        return false;
    }

    bool is_foreign_window(::HWND hWnd) noexcept
    {
        if (!hWnd) return false;
        ::DWORD pid = 0;
        ::GetWindowThreadProcessId(hWnd, &pid);
        return pid != 0 && pid != ::GetCurrentProcessId();
    }

    namespace hooks
    {
        windower::hooklib::hook<decltype(::FindWindowA)> FindWindowA;
        windower::hooklib::hook<decltype(::FindWindowW)> FindWindowW;
        windower::hooklib::hook<decltype(::FindWindowExA)> FindWindowExA;
        windower::hooklib::hook<decltype(::FindWindowExW)> FindWindowExW;
        windower::hooklib::hook<decltype(::EnumWindows)> EnumWindows;
        windower::hooklib::hook<decltype(::GetClassNameA)> GetClassNameA;
        windower::hooklib::hook<decltype(::GetClassNameW)> GetClassNameW;
        windower::hooklib::hook<decltype(::GetWindowTextA)> GetWindowTextA;
        windower::hooklib::hook<decltype(::GetWindowTextW)> GetWindowTextW;
    }

    namespace callbacks
    {
        ::HWND WINAPI FindWindowA(::LPCSTR lpClassName, ::LPCSTR lpWindowName) noexcept
        {
            ::HWND const hwnd = hooks::FindWindowA(lpClassName, lpWindowName);
            if (hwnd && is_game_window(lpClassName, lpWindowName) && is_foreign_window(hwnd)) return nullptr;
            return hwnd;
        }

        ::HWND WINAPI FindWindowW(::LPCWSTR lpClassName, ::LPCWSTR lpWindowName) noexcept
        {
            ::HWND const hwnd = hooks::FindWindowW(lpClassName, lpWindowName);
            if (hwnd && is_game_windowW(lpClassName, lpWindowName) && is_foreign_window(hwnd)) return nullptr;
            return hwnd;
        }

        ::HWND WINAPI FindWindowExA(::HWND hWndParent, ::HWND hWndChildAfter, ::LPCSTR lpClassName, ::LPCSTR lpWindowName) noexcept
        {
            ::HWND const hwnd = hooks::FindWindowExA(hWndParent, hWndChildAfter, lpClassName, lpWindowName);
            if (hwnd && is_game_window(lpClassName, lpWindowName) && is_foreign_window(hwnd)) return nullptr;
            return hwnd;
        }

        ::HWND WINAPI FindWindowExW(::HWND hWndParent, ::HWND hWndChildAfter, ::LPCWSTR lpClassName, ::LPCWSTR lpWindowName) noexcept
        {
            ::HWND const hwnd = hooks::FindWindowExW(hWndParent, hWndChildAfter, lpClassName, lpWindowName);
            if (hwnd && is_game_windowW(lpClassName, lpWindowName) && is_foreign_window(hwnd)) return nullptr;
            return hwnd;
        }

        struct enum_ctx
        {
            ::WNDENUMPROC func;
            ::LPARAM param;
        };

        bool is_hidden_target(::HWND hwnd) noexcept
        {
            if (!is_foreign_window(hwnd)) return false;
            char cls[256] = { 0 };
            hooks::GetClassNameA ? hooks::GetClassNameA(hwnd, cls, 256) : ::GetClassNameA(hwnd, cls, 256);
            char title[256] = { 0 };
            hooks::GetWindowTextA ? hooks::GetWindowTextA(hwnd, title, 256) : ::GetWindowTextA(hwnd, title, 256);
            return is_game_window(cls, title);
        }

        ::BOOL CALLBACK enum_windows_filter(::HWND hwnd, ::LPARAM lParam) noexcept
        {
            if (is_hidden_target(hwnd)) return TRUE;
            auto const* const ctx = reinterpret_cast<const enum_ctx*>(lParam);
            return ctx->func(hwnd, ctx->param);
        }

        ::BOOL WINAPI EnumWindows(::WNDENUMPROC lpEnumFunc, ::LPARAM lParam) noexcept
        {
            enum_ctx ctx{ lpEnumFunc, lParam };
            return hooks::EnumWindows(enum_windows_filter, reinterpret_cast<::LPARAM>(&ctx));
        }

        int WINAPI GetClassNameA(::HWND hWnd, ::LPSTR lpClassName, int nMaxCount) noexcept
        {
            int const ret = hooks::GetClassNameA(hWnd, lpClassName, nMaxCount);
            if (ret > 0 && is_game_window(lpClassName, nullptr) && is_foreign_window(hWnd))
            {
                strncpy_s(lpClassName, nMaxCount, "HiddenClass", _TRUNCATE);
                return 11;
            }
            return ret;
        }

        int WINAPI GetClassNameW(::HWND hWnd, ::LPWSTR lpClassName, int nMaxCount) noexcept
        {
            int const ret = hooks::GetClassNameW(hWnd, lpClassName, nMaxCount);
            if (ret > 0 && is_foreign_window(hWnd))
            {
                char mb_name[256] = { 0 };
                ::WideCharToMultiByte(CP_ACP, 0, lpClassName, -1, mb_name, 256, nullptr, nullptr);
                if (is_game_window(mb_name, nullptr))
                {
                    wcsncpy_s(lpClassName, nMaxCount, L"HiddenClass", _TRUNCATE);
                    return 11;
                }
            }
            return ret;
        }

        int WINAPI GetWindowTextA(::HWND hWnd, ::LPSTR lpString, int nMaxCount) noexcept
        {
            int const ret = hooks::GetWindowTextA(hWnd, lpString, nMaxCount);
            if (ret > 0 && is_foreign_window(hWnd) && is_game_window(nullptr, lpString))
            {
                strncpy_s(lpString, nMaxCount, "HiddenWindow", _TRUNCATE);
                return 12;
            }
            return ret;
        }

        int WINAPI GetWindowTextW(::HWND hWnd, ::LPWSTR lpString, int nMaxCount) noexcept
        {
            int const ret = hooks::GetWindowTextW(hWnd, lpString, nMaxCount);
            if (ret > 0 && is_foreign_window(hWnd))
            {
                char mb_name[256] = { 0 };
                ::WideCharToMultiByte(CP_ACP, 0, lpString, -1, mb_name, 256, nullptr, nullptr);
                if (is_game_window(nullptr, mb_name))
                {
                    wcsncpy_s(lpString, nMaxCount, L"HiddenWindow", _TRUNCATE);
                    return 12;
                }
            }
            return ret;
        }
    }
}

void windower::user32::install_window_find()
{
    // FORCE user32.dll to fully map and initialize in memory. 
    // APC injection runs early; if user32 isn't ready, make_hook silently fails!
    ::LoadLibraryA("user32.dll");

    if (!hooks::FindWindowA)
    {
        hooks::FindWindowA = hooklib::make_hook(u8"user32.dll", u8"FindWindowA", callbacks::FindWindowA);
        hooks::FindWindowW = hooklib::make_hook(u8"user32.dll", u8"FindWindowW", callbacks::FindWindowW);
        hooks::FindWindowExA = hooklib::make_hook(u8"user32.dll", u8"FindWindowExA", callbacks::FindWindowExA);
        hooks::FindWindowExW = hooklib::make_hook(u8"user32.dll", u8"FindWindowExW", callbacks::FindWindowExW);
        hooks::EnumWindows = hooklib::make_hook(u8"user32.dll", u8"EnumWindows", callbacks::EnumWindows);
        hooks::GetClassNameA = hooklib::make_hook(u8"user32.dll", u8"GetClassNameA", callbacks::GetClassNameA);
        hooks::GetClassNameW = hooklib::make_hook(u8"user32.dll", u8"GetClassNameW", callbacks::GetClassNameW);
        hooks::GetWindowTextA = hooklib::make_hook(u8"user32.dll", u8"GetWindowTextA", callbacks::GetWindowTextA);
        hooks::GetWindowTextW = hooklib::make_hook(u8"user32.dll", u8"GetWindowTextW", callbacks::GetWindowTextW);
    }
}

void windower::user32::uninstall_window_find() noexcept
{
    hooks::FindWindowA = {};
    hooks::FindWindowW = {};
    hooks::FindWindowExA = {};
    hooks::FindWindowExW = {};
    hooks::EnumWindows = {};
    hooks::GetClassNameA = {};
    hooks::GetClassNameW = {};
    hooks::GetWindowTextA = {};
    hooks::GetWindowTextW = {};
}
