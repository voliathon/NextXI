#include "user32_window_split.hpp"
#include "hooklib/hook.hpp"

namespace
{
    namespace hooks
    {
        windower::hooklib::hook<decltype(::RegisterClassA)> RegisterClassA;
        windower::hooklib::hook<decltype(::RegisterClassExW)> RegisterClassExW;
        windower::hooklib::hook<decltype(::GetClassLongA)> GetClassLongA;
        windower::hooklib::hook<decltype(::GetClassLongW)> GetClassLongW;
        windower::hooklib::hook<decltype(::SetClassLongA)> SetClassLongA;
        windower::hooklib::hook<decltype(::SetClassLongW)> SetClassLongW;
        windower::hooklib::hook<decltype(::GetWindowLongA)> GetWindowLongA;
        windower::hooklib::hook<decltype(::GetWindowLongW)> GetWindowLongW;
        windower::hooklib::hook<decltype(::SetWindowLongA)> SetWindowLongA;
        windower::hooklib::hook<decltype(::SetWindowLongW)> SetWindowLongW;
    }

    namespace callbacks
    {
        ::ATOM WINAPI RegisterClassA(::WNDCLASSA const* lpWndClass) noexcept
        {
            if (windower::user32::check_class_name(lpWndClass->lpszClassName, "FFXiClass"))
            {
                auto window_class = *lpWndClass;
                window_class.cbWndExtra += windower::user32::window_data_size;
                auto const result = hooks::RegisterClassA(&window_class);
                windower::user32::ffxi_class_atom = result;
                return result;
            }
            return hooks::RegisterClassA(lpWndClass);
        }

        ::ATOM WINAPI RegisterClassExW(::WNDCLASSEXW const* lpWndClass) noexcept
        {
            if (windower::user32::check_class_name(lpWndClass->lpszClassName, L"PlayOnlineUS") ||
                windower::user32::check_class_name(lpWndClass->lpszClassName, L"PlayOnlineEU") ||
                windower::user32::check_class_name(lpWndClass->lpszClassName, L"PlayOnlineJP"))
            {
                auto const result = hooks::RegisterClassExW(lpWndClass);
                windower::user32::pol_class_atom = result;
                return result;
            }
            return hooks::RegisterClassExW(lpWndClass);
        }

        ::DWORD WINAPI GetClassLongA(::HWND hWnd, int nIndex) noexcept
        {
            if (nIndex == GCL_CBWNDEXTRA && windower::user32::check_class(hWnd, windower::user32::ffxi_class_atom))
                return hooks::GetClassLongA(hWnd, nIndex) - windower::user32::window_data_size;
            return hooks::GetClassLongA(hWnd, nIndex);
        }

        ::DWORD WINAPI GetClassLongW(::HWND hWnd, int nIndex) noexcept
        {
            if (nIndex == GCL_CBWNDEXTRA && windower::user32::check_class(hWnd, windower::user32::ffxi_class_atom))
                return hooks::GetClassLongW(hWnd, nIndex) - windower::user32::window_data_size;
            return hooks::GetClassLongW(hWnd, nIndex);
        }

        ::DWORD WINAPI SetClassLongA(::HWND hWnd, int nIndex, ::LONG dwNewLong) noexcept
        {
            if (nIndex == GCL_CBWNDEXTRA && windower::user32::check_class(hWnd, windower::user32::ffxi_class_atom))
            {
                auto const new_value = dwNewLong + windower::user32::window_data_size;
                auto const result = hooks::SetClassLongA(hWnd, nIndex, new_value);
                return result - windower::user32::window_data_size;
            }
            return hooks::SetClassLongA(hWnd, nIndex, dwNewLong);
        }

        ::DWORD WINAPI SetClassLongW(::HWND hWnd, int nIndex, ::LONG dwNewLong) noexcept
        {
            if (nIndex == GCL_CBWNDEXTRA && windower::user32::check_class(hWnd, windower::user32::ffxi_class_atom))
            {
                auto const new_value = dwNewLong + windower::user32::window_data_size;
                auto const result = hooks::SetClassLongA(hWnd, nIndex, new_value);
                return result - windower::user32::window_data_size;
            }
            return hooks::SetClassLongW(hWnd, nIndex, dwNewLong);
        }

        ::LONG WINAPI GetWindowLongA(::HWND hWnd, int nIndex) noexcept
        {
            if (nIndex >= 0 || nIndex == GWL_WNDPROC)
            {
                if (windower::user32::check_class(hWnd, windower::user32::ffxi_class_atom))
                {
                    if (nIndex >= 0) nIndex += windower::user32::window_data_size;
                    else if (nIndex == GWL_WNDPROC)
                    {
                        if (auto const* const data = windower::user32::get_window_data(hWnd)) return std::bit_cast<::LONG>(data->wnd_proc_a);
                    }
                }
            }
            return hooks::GetWindowLongA(hWnd, nIndex);
        }

        ::LONG WINAPI GetWindowLongW(::HWND hWnd, int nIndex) noexcept
        {
            if (nIndex >= 0 || nIndex == GWL_WNDPROC)
            {
                if (windower::user32::check_class(hWnd, windower::user32::ffxi_class_atom))
                {
                    if (nIndex >= 0) nIndex += windower::user32::window_data_size;
                    else if (nIndex == GWL_WNDPROC)
                    {
                        if (auto const* const data = windower::user32::get_window_data(hWnd)) return std::bit_cast<::LONG>(data->wnd_proc_w);
                    }
                }
            }
            return hooks::GetWindowLongW(hWnd, nIndex);
        }

        ::LONG WINAPI SetWindowLongA(::HWND hWnd, int nIndex, ::LONG dwNewLong) noexcept
        {
            if (nIndex == GWL_STYLE || nIndex >= 0 || nIndex == GWL_WNDPROC)
            {
                if (windower::user32::check_class(hWnd, windower::user32::ffxi_class_atom))
                {
                    if (nIndex == GWL_STYLE) return hooks::GetWindowLongA(hWnd, GWL_STYLE);
                    if (nIndex >= 0) nIndex += windower::user32::window_data_size;
                    else if (nIndex == GWL_WNDPROC)
                    {
                        if (auto const* const data = windower::user32::get_window_data(hWnd))
                        {
                            auto wnd_proc = std::bit_cast<::LONG>(data->wnd_proc);
                            hooks::SetWindowLongW(hWnd, GWL_WNDPROC, wnd_proc);
                            auto result = hooks::SetWindowLongA(hWnd, GWL_WNDPROC, dwNewLong);
                            wnd_proc = hooks::SetWindowLongW(hWnd, GWL_WNDPROC, wnd_proc);
                            const_cast<windower::user32::window_data*>(data)->wnd_proc = std::bit_cast<::WNDPROC>(wnd_proc);
                            return result;
                        }
                    }
                }
            }
            return hooks::SetWindowLongA(hWnd, nIndex, dwNewLong);
        }

        ::LONG WINAPI SetWindowLongW(::HWND hWnd, int nIndex, ::LONG dwNewLong) noexcept
        {
            if (nIndex == GWL_STYLE || nIndex >= 0 || nIndex == GWL_WNDPROC)
            {
                if (windower::user32::check_class(hWnd, windower::user32::ffxi_class_atom))
                {
                    if (nIndex == GWL_STYLE) return hooks::GetWindowLongW(hWnd, GWL_STYLE);
                    if (nIndex >= 0) nIndex += windower::user32::window_data_size;
                    else if (nIndex == GWL_WNDPROC)
                    {
                        if (auto const* const data = windower::user32::get_window_data(hWnd))
                        {
                            auto wnd_proc = std::bit_cast<::LONG>(data->wnd_proc);
                            hooks::SetWindowLongW(hWnd, GWL_WNDPROC, wnd_proc);
                            auto result = hooks::SetWindowLongW(hWnd, GWL_WNDPROC, dwNewLong);
                            wnd_proc = hooks::SetWindowLongW(hWnd, GWL_WNDPROC, wnd_proc);
                            const_cast<windower::user32::window_data*>(data)->wnd_proc = std::bit_cast<::WNDPROC>(wnd_proc);
                            return result;
                        }
                    }
                }
            }
            return hooks::SetWindowLongW(hWnd, nIndex, dwNewLong);
        }
    }
}

windower::user32::window_data* windower::user32::get_window_data(::HWND hwnd) noexcept
{
    return std::bit_cast<window_data*>(hooks::GetWindowLongW ? hooks::GetWindowLongW(hwnd, 0) : ::GetWindowLongW(hwnd, 0));
}

void windower::user32::set_window_data(::HWND hwnd, window_data* ptr) noexcept
{
    if (hooks::SetWindowLongW) hooks::SetWindowLongW(hwnd, 0, std::bit_cast<::LONG>(ptr));
}

::LONG windower::user32::set_window_wndproc(::HWND hwnd, ::LONG wnd_proc) noexcept
{
    return hooks::SetWindowLongW ? hooks::SetWindowLongW(hwnd, GWL_WNDPROC, wnd_proc) : 0;
}

void windower::user32::install_window_class()
{
    hooks::RegisterClassA = hooklib::make_hook(u8"user32.dll", u8"RegisterClassA", callbacks::RegisterClassA);
    hooks::RegisterClassExW = hooklib::make_hook(u8"user32.dll", u8"RegisterClassExW", callbacks::RegisterClassExW);
    hooks::GetClassLongA = hooklib::make_hook(u8"user32.dll", u8"GetClassLongA", callbacks::GetClassLongA);
    hooks::GetClassLongW = hooklib::make_hook(u8"user32.dll", u8"GetClassLongW", callbacks::GetClassLongW);
    hooks::SetClassLongA = hooklib::make_hook(u8"user32.dll", u8"SetClassLongA", callbacks::SetClassLongA);
    hooks::SetClassLongW = hooklib::make_hook(u8"user32.dll", u8"SetClassLongW", callbacks::SetClassLongW);
    hooks::GetWindowLongA = hooklib::make_hook(u8"user32.dll", u8"GetWindowLongA", callbacks::GetWindowLongA);
    hooks::GetWindowLongW = hooklib::make_hook(u8"user32.dll", u8"GetWindowLongW", callbacks::GetWindowLongW);
    hooks::SetWindowLongA = hooklib::make_hook(u8"user32.dll", u8"SetWindowLongA", callbacks::SetWindowLongA);
    hooks::SetWindowLongW = hooklib::make_hook(u8"user32.dll", u8"SetWindowLongW", callbacks::SetWindowLongW);
}

void windower::user32::uninstall_window_class() noexcept
{
    hooks::RegisterClassA = {}; hooks::RegisterClassExW = {};
    hooks::GetClassLongA = {}; hooks::GetClassLongW = {};
    hooks::SetClassLongA = {}; hooks::SetClassLongW = {};
    hooks::GetWindowLongA = {}; hooks::GetWindowLongW = {};
    hooks::SetWindowLongA = {}; hooks::SetWindowLongW = {};
}
