#include "user32_window_split.hpp"
#include "user32_internal.hpp"
#include "hooks/user32.hpp"
#include "hooklib/hook.hpp"
#include "core.hpp"
#include "library.hpp"
#include "utilities/paths.hpp"
#include "utilities/module_info.hpp"

#include <propsys.h>
#include <propkey.h>
#include <propvarutil.h>
#include <windowsx.h>
#include <winrt/base.h>

namespace windower::user32
{
    ::ATOM ffxi_class_atom = 0;
    ::ATOM pol_class_atom = 0;

    ::HWND pol_hwnd_cache = nullptr;
    ::HWND ffxi_hwnd_cache = nullptr;

    std::basic_string_view<::CHAR> get_atom_name(::ATOM atom, std::span<::CHAR> const buffer) noexcept
    {
        return { buffer.data(), ::GetAtomNameA(atom, buffer.data(), buffer.size()) };
    }

    std::basic_string_view<::WCHAR> get_atom_name(::ATOM atom, std::span<::WCHAR> const buffer) noexcept
    {
        return { buffer.data(), ::GetAtomNameW(atom, buffer.data(), buffer.size()) };
    }

    bool check_class(::HWND hwnd, ATOM expected) noexcept
    {
        auto const atom = ::GetClassLongW(hwnd, GCW_ATOM);
        return atom && atom == expected;
    }

    void set_window_properties(::HWND hwnd)
    {
        if (windower::library shell32{ u8"shell32.dll" })
        {
            if (auto ptr = shell32.get_function(u8"SHGetPropertyStoreForWindow"))
            {
                using SHGetPropertyStoreForWindow = ::HRESULT(STDAPICALLTYPE*)(::HWND, ::IID const&, void**);
                winrt::com_ptr<::IPropertyStore> properties;
                if (SUCCEEDED(reinterpret_cast<SHGetPropertyStoreForWindow>(ptr)(hwnd, IID_PPV_ARGS(properties.put()))))
                {
                    GSL_SUPPRESS("type.7")
                    {
                        ::PROPVARIANT value{};
                        if (SUCCEEDED(::InitPropVariantFromString(L"Windower.Windower", &value))) {
                            properties->SetValue(PKEY_AppUserModel_ID, value);
                            ::PropVariantClear(&value);
                        }
                        auto executable = windower::windower_path() / u8"windower.exe";
                        if (SUCCEEDED(::InitPropVariantFromString(executable.c_str(), &value))) {
                            properties->SetValue(PKEY_AppUserModel_RelaunchCommand, value);
                            ::PropVariantClear(&value);
                        }
                        if (SUCCEEDED(::InitPropVariantFromString(L"Windower", &value))) {
                            properties->SetValue(PKEY_AppUserModel_RelaunchDisplayNameResource, value);
                            ::PropVariantClear(&value);
                        }
                    }
                }
            }
        }
    }

    ::LRESULT CALLBACK ffxi_wnd_proc(::HWND hwnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam) noexcept
    {
        auto const data = get_window_data(hwnd);
        if (uMsg == WM_NCDESTROY)
        {
            ::SetWindowLongW(hwnd, 0, 0);
            auto const wnd_proc = data->wnd_proc;
            std::unique_ptr<window_data>{data}.reset();
            return ::CallWindowProcW(wnd_proc, hwnd, uMsg, wParam, lParam);
        }
        else
        {
            auto const time = gsl::narrow_cast<::DWORD>(::GetMessageTime());
            auto const pos = ::GetMessagePos();
            auto const x = GET_X_LPARAM(pos);
            auto const y = GET_Y_LPARAM(pos);
            if (data->current.hwnd != hwnd || data->current.message != uMsg ||
                data->current.wParam != wParam || data->current.lParam != lParam ||
                data->current.pt.x != x || data->current.pt.y != y || data->current.time != time)
            {
                auto& core = windower::core::instance();
                if (auto const result = core.process_message({
                        .hwnd = hwnd, .message = uMsg, .wParam = wParam, .lParam = lParam,
                        .time = time, .pt = {.x = x, .y = y},
                    }))
                {
                    return *result;
                }
            }
            else { data->current = {}; }
        }

        switch (uMsg)
        {
        default: break;
        case WM_WINDOWPOSCHANGING:
        {
            auto const pWindowPos = std::bit_cast<LPWINDOWPOS>(lParam);
            if (pWindowPos && !(pWindowPos->flags & SWP_NOMOVE)) pWindowPos->flags &= ~SWP_NOCOPYBITS;
            break;
        }
        case WM_STYLECHANGING:
        {
            if (wParam == GWL_STYLE)
            {
                auto const pStyle = std::bit_cast<LPSTYLESTRUCT>(lParam);
                if (pStyle)
                {
                    pStyle->styleNew |= (WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
                    pStyle->styleNew &= ~WS_POPUP;
                }
                return 0;
            }
            break;
        }
        case WM_ACTIVATE:
            windower::user32::set_process_muted(LOWORD(wParam) == WA_INACTIVE);
            break;
        case WM_ACTIVATEAPP:
            windower::user32::set_process_muted(wParam == FALSE);
            break;
        case WM_SYSCOMMAND:
        {
            auto const cmd = wParam & 0xFFF0;
            if (cmd == SC_MINIMIZE) windower::user32::set_process_muted(true);
            break;
        }
        case WM_SIZE:
        {
            if (wParam == SIZE_MINIMIZED) windower::user32::set_process_muted(true);
            break;
        }
        case WM_SETTEXT:
            if (data->update_title) { data->update_title = false; break; }
            data->title = lParam ? std::bit_cast<::WCHAR*>(lParam) : L"";
            return TRUE;
        case WM_GETTEXT:
            if (lParam) {
                std::fill_n(std::bit_cast<::WCHAR*>(lParam), wParam, 0);
                return data->title.copy(std::bit_cast<::WCHAR*>(lParam), wParam);
            }
            return 0;
        case WM_GETTEXTLENGTH: return data->title.size();
        }

        return ::CallWindowProcW(data->wnd_proc, hwnd, uMsg, wParam, lParam);
    }
}
