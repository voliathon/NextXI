#include "user32_window_split.hpp"
#include "hooklib/hook.hpp"
#include "core.hpp"
#include "resource.hpp"
#include "utilities/module_info.hpp"
#include <memory>
#include <cstring>
#include <cwchar>

namespace
{
    namespace hooks
    {
        windower::hooklib::hook<decltype(::CreateDialogParamW)> CreateDialogParamW;
        windower::hooklib::hook<decltype(::LoadIconA)> LoadIconA;
        windower::hooklib::hook<decltype(::LoadIconW)> LoadIconW;
        windower::hooklib::hook<decltype(::CreateWindowExA)> CreateWindowExA;
        windower::hooklib::hook<decltype(::CreateWindowExW)> CreateWindowExW;
        windower::hooklib::hook<decltype(::MoveWindow)> MoveWindow;
        windower::hooklib::hook<decltype(::SetWindowPos)> SetWindowPos;
        windower::hooklib::hook<decltype(::SetWindowTextA)> SetWindowTextA;
        windower::hooklib::hook<decltype(::SetWindowTextW)> SetWindowTextW;
    }

    namespace callbacks
    {
        ::HWND WINAPI CreateDialogParamW(::HINSTANCE hInstance, ::LPCWSTR lpTemplateName, ::HWND hWndParent, ::DLGPROC lpDialogFunc, ::LPARAM dwInitParam) noexcept
        {
            auto const hwnd = hooks::CreateDialogParamW(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
            if (hInstance == ::GetModuleHandleW(nullptr) && std::bit_cast<std::uintptr_t>(lpTemplateName) == 103) ::ShowWindow(hwnd, SW_HIDE);
            return hwnd;
        }

        ::HICON WINAPI LoadIconA(::HINSTANCE hInstance, ::LPCSTR lpIconName) noexcept
        {
            if (hInstance || (IS_INTRESOURCE(lpIconName) && std::bit_cast<::ULONG_PTR>(lpIconName) >= 0x7F00))
            {
                hInstance = static_cast<::HINSTANCE>(windower::windower_module());
                lpIconName = MAKEINTRESOURCEA(ICON_NEXTXI);
            }
            return hooks::LoadIconA(hInstance, lpIconName);
        }

        ::HICON WINAPI LoadIconW(::HINSTANCE hInstance, ::LPCWSTR lpIconName) noexcept
        {
            if (hInstance || (IS_INTRESOURCE(lpIconName) && std::bit_cast<::ULONG_PTR>(lpIconName) >= 0x7F00))
            {
                hInstance = static_cast<::HINSTANCE>(windower::windower_module());
                lpIconName = MAKEINTRESOURCEW(ICON_NEXTXI);
            }
            return hooks::LoadIconW(hInstance, lpIconName);
        }

        ::HWND WINAPI CreateWindowExA(::DWORD dwExStyle, ::LPCSTR lpClassName, ::LPCSTR lpWindowName, ::DWORD dwStyle, int x, int y, int nWidth, int nHeight, ::HWND hWndParent, ::HMENU hMenu, ::HINSTANCE hInstance, ::LPVOID lpParam) noexcept
        {
            auto hwnd = hooks::CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

            if (hwnd && windower::user32::check_class(hwnd, windower::user32::ffxi_class_atom))
            {
                auto ptr = std::make_unique<windower::user32::window_data>();
                auto data = ptr.get();
                windower::user32::set_window_data(hwnd, ptr.release());

                auto const wnd_proc = windower::user32::set_window_wndproc(hwnd, std::bit_cast<::LONG>(&windower::user32::ffxi_wnd_proc));
                data->wnd_proc = std::bit_cast<::WNDPROC>(wnd_proc);

                auto const title_size = std::strlen(lpWindowName);
                data->title.resize(::MultiByteToWideChar(CP_ACP, 0, lpWindowName, title_size, nullptr, 0));
                ::MultiByteToWideChar(CP_ACP, 0, lpWindowName, title_size, data->title.data(), data->title.size());

                ::SetWindowTextW(hwnd, L"Next XI");
                windower::user32::set_window_properties(hwnd);

                auto const hIcon = hooks::LoadIconW(static_cast<::HINSTANCE>(windower::windower_module()), MAKEINTRESOURCEW(ICON_NEXTXI));
                ::SendMessageW(hwnd, WM_SETICON, ICON_BIG, std::bit_cast<::LPARAM>(hIcon));
                ::SendMessageW(hwnd, WM_SETICON, ICON_SMALL, std::bit_cast<::LPARAM>(hIcon));
                ::SetClassLongPtrW(hwnd, GCLP_HICON, std::bit_cast<::LONG_PTR>(hIcon));
                ::SetClassLongPtrW(hwnd, GCLP_HICONSM, std::bit_cast<::LONG_PTR>(hIcon));

                windower::core::instance().client_hwnd = hwnd;
            }
            return hwnd;
        }

        ::HWND WINAPI CreateWindowExW(::DWORD dwExStyle, ::LPCWSTR lpClassName, ::LPCWSTR lpWindowName, ::DWORD dwStyle, int x, int y, int nWidth, int nHeight, ::HWND hWndParent, ::HMENU hMenu, ::HINSTANCE hInstance, ::LPVOID lpParam) noexcept
        {
            auto hwnd = hooks::CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
            if (hwnd && windower::user32::check_class(hwnd, windower::user32::pol_class_atom)) {
                windower::user32::set_window_properties(hwnd);
            }
            return hwnd;
        }

        ::BOOL WINAPI MoveWindow(::HWND hWnd, int X, int Y, int nWidth, int nHeight, ::BOOL bRepaint) noexcept
        {
            if (windower::is_game_module(WINDOWER_RETURN_ADDRESS)) return TRUE;
            return hooks::MoveWindow(hWnd, X, Y, nWidth, nHeight, bRepaint);
        }

        ::BOOL WINAPI SetWindowPos(::HWND hWnd, ::HWND hWndInsertAfter, int X, int Y, int cx, int cy, ::UINT uFlags) noexcept
        {
            if (windower::is_game_module(WINDOWER_RETURN_ADDRESS)) return TRUE;
            return hooks::SetWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
        }

        ::BOOL WINAPI SetWindowTextA(::HWND hWnd, ::LPCSTR lpString) noexcept
        {
            if (!windower::is_game_module(WINDOWER_RETURN_ADDRESS))
                if (auto const data = windower::user32::get_window_data(hWnd)) data->update_title = true;
            return hooks::SetWindowTextA(hWnd, lpString);
        }

        ::BOOL WINAPI SetWindowTextW(::HWND hWnd, ::LPCWSTR lpString) noexcept
        {
            if (!windower::is_game_module(WINDOWER_RETURN_ADDRESS))
                if (auto const data = windower::user32::get_window_data(hWnd)) data->update_title = true;
            return hooks::SetWindowTextW(hWnd, lpString);
        }
    }
}

void windower::user32::install_window_create()
{
    hooks::CreateDialogParamW = hooklib::make_hook(u8"user32.dll", u8"CreateDialogParamW", callbacks::CreateDialogParamW);
    hooks::LoadIconA = hooklib::make_hook(u8"user32.dll", u8"LoadIconA", callbacks::LoadIconA);
    hooks::LoadIconW = hooklib::make_hook(u8"user32.dll", u8"LoadIconW", callbacks::LoadIconW);
    hooks::CreateWindowExA = hooklib::make_hook(u8"user32.dll", u8"CreateWindowExA", callbacks::CreateWindowExA);
    hooks::CreateWindowExW = hooklib::make_hook(u8"user32.dll", u8"CreateWindowExW", callbacks::CreateWindowExW);
    hooks::MoveWindow = hooklib::make_hook(u8"user32.dll", u8"MoveWindow", callbacks::MoveWindow);
    hooks::SetWindowPos = hooklib::make_hook(u8"user32.dll", u8"SetWindowPos", callbacks::SetWindowPos);
    hooks::SetWindowTextA = hooklib::make_hook(u8"user32.dll", u8"SetWindowTextA", callbacks::SetWindowTextA);
    hooks::SetWindowTextW = hooklib::make_hook(u8"user32.dll", u8"SetWindowTextW", callbacks::SetWindowTextW);
}

void windower::user32::uninstall_window_create() noexcept
{
    hooks::CreateDialogParamW = {}; hooks::LoadIconA = {}; hooks::LoadIconW = {};
    hooks::CreateWindowExA = {}; hooks::CreateWindowExW = {};
    hooks::MoveWindow = {}; hooks::SetWindowPos = {};
    hooks::SetWindowTextA = {}; hooks::SetWindowTextW = {};
}
