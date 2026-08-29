#include "hooks/kernel32.hpp"
#include "kernel32_internal.hpp"
#include "hooks/user32_window_find.hpp"
#include "cloak.hpp"
#include "hooklib/hook.hpp"
#include "utilities/module_info.hpp"
#include <windows.h>
#include <bit>
#include <cwchar>

namespace
{
    ::LPTOP_LEVEL_EXCEPTION_FILTER unhandled_exception_filter = nullptr;

    namespace hooks
    {
        windower::hooklib::hook<decltype(::GetACP)> GetACP;
        windower::hooklib::hook<decltype(::LoadLibraryW)> LoadLibraryW;
        windower::hooklib::hook<decltype(::SetUnhandledExceptionFilter)> SetUnhandledExceptionFilter;
    }

    namespace callbacks
    {
        ::UINT WINAPI GetACP() noexcept { return 932; }

        ::HMODULE WINAPI LoadLibraryW(::LPCWSTR lpFileName) noexcept
        {
            if (lpFileName && ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, lpFileName, -1, L"hook.dll", 8) == CSTR_EQUAL)
            {
                return nullptr;
            }
            else if (!windower::is_game_module(WINDOWER_RETURN_ADDRESS))
            {
                auto guard = windower::uncloak();
                return hooks::LoadLibraryW(lpFileName);
            }
            return hooks::LoadLibraryW(lpFileName);
        }

        ::LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilter(::LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) noexcept
        {
            return std::bit_cast<::LPTOP_LEVEL_EXCEPTION_FILTER>(
                ::InterlockedExchangePointer(std::bit_cast<void**>(&unhandled_exception_filter), static_cast<void*>(lpTopLevelExceptionFilter)));
        }
    }
}

::UINT windower::kernel32::GetACP() noexcept
{
    return hooks::GetACP ? hooks::GetACP() : ::GetACP();
}

::LPTOP_LEVEL_EXCEPTION_FILTER windower::kernel32::SetUnhandledExceptionFilter(::LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) noexcept
{
    return hooks::SetUnhandledExceptionFilter ? hooks::SetUnhandledExceptionFilter(lpTopLevelExceptionFilter) : ::SetUnhandledExceptionFilter(lpTopLevelExceptionFilter);
}

void windower::kernel32::install_misc()
{
    if (!hooks::GetACP)
    {
        hooks::GetACP = hooklib::make_hook(u8"kernel32.dll", u8"GetACP", callbacks::GetACP);
        hooks::LoadLibraryW = hooklib::make_hook(u8"kernel32.dll", u8"LoadLibraryW", callbacks::LoadLibraryW);
        hooks::SetUnhandledExceptionFilter = hooklib::make_hook(u8"kernel32.dll", u8"SetUnhandledExceptionFilter", callbacks::SetUnhandledExceptionFilter);
    }
}

void windower::kernel32::uninstall_misc() noexcept
{
    hooks::GetACP = {};
    hooks::LoadLibraryW = {};
    hooks::SetUnhandledExceptionFilter = {};
}
