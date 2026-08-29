#include "hooks/kernel32.hpp"
#include "kernel32_internal.hpp"
#include "hooklib/hook.hpp"
#include "unicode.hpp"

#include <windows.h>
#include <cstdio>
#include <string>
#include <string_view>
#include <cwctype>
#include <algorithm>

namespace
{
    thread_local bool t_in_hook = false;

    std::wstring get_isolated_name(::LPCWSTR lpName)
    {
        std::wstring name = lpName;
        std::wstring lower_name = name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), std::towlower);

        // Aggressive case-insensitive substring isolation
        if (lower_name.find(L"playonline") != std::wstring::npos ||
            lower_name.find(L"pol") != std::wstring::npos ||
            lower_name.find(L"ffxi") != std::wstring::npos ||
            lower_name.find(L"final fantasy") != std::wstring::npos ||
            lower_name.find(L"sqpocon") != std::wstring::npos ||
            lower_name.find(L"irc0") != std::wstring::npos ||
            lower_name.find(L"profdb") != std::wstring::npos ||
            lower_name.find(L"privatedata") != std::wstring::npos ||
            lower_name.find(L"messanger") != std::wstring::npos ||
            lower_name.find(L"cddf2128") != std::wstring::npos ||
            lower_name.find(L"fpi") != std::wstring::npos ||
            lower_name.find(L"bpi") != std::wstring::npos ||
            lower_name.find(L"fca") != std::wstring::npos ||
            lower_name.find(L"bca") != std::wstring::npos ||
            lower_name.find(L"fei") != std::wstring::npos ||
            lower_name.find(L"bei") != std::wstring::npos ||
            lower_name.find(L"fha") != std::wstring::npos ||
            lower_name.find(L"bha") != std::wstring::npos ||
            lower_name.find(L"sq_") != std::wstring::npos ||
            lower_name.find(L"generalmail") != std::wstring::npos)
        {
            name.append(L"_").append(std::to_wstring(::GetCurrentProcessId()));
        }
        return name;
    }

    namespace hooks
    {
        windower::hooklib::hook<decltype(::CreateMutexW)> CreateMutexW;
        windower::hooklib::hook<decltype(::OpenMutexW)> OpenMutexW;
        windower::hooklib::hook<decltype(::CreateMutexA)> CreateMutexA;
        windower::hooklib::hook<decltype(::OpenMutexA)> OpenMutexA;

        windower::hooklib::hook<decltype(::CreateEventW)> CreateEventW;
        windower::hooklib::hook<decltype(::CreateEventA)> CreateEventA;
        windower::hooklib::hook<decltype(::OpenEventW)> OpenEventW;
        windower::hooklib::hook<decltype(::OpenEventA)> OpenEventA;

        windower::hooklib::hook<decltype(::CreateSemaphoreW)> CreateSemaphoreW;
        windower::hooklib::hook<decltype(::CreateSemaphoreA)> CreateSemaphoreA;
        windower::hooklib::hook<decltype(::OpenSemaphoreW)> OpenSemaphoreW;
        windower::hooklib::hook<decltype(::OpenSemaphoreA)> OpenSemaphoreA;

        windower::hooklib::hook<decltype(::CreateFileMappingW)> CreateFileMappingW;
        windower::hooklib::hook<decltype(::CreateFileMappingA)> CreateFileMappingA;
        windower::hooklib::hook<decltype(::OpenFileMappingW)> OpenFileMappingW;
        windower::hooklib::hook<decltype(::OpenFileMappingA)> OpenFileMappingA;
    }

    namespace callbacks
    {
        // Mutexes
        ::HANDLE WINAPI CreateMutexW(::LPSECURITY_ATTRIBUTES lpMutexAttributes, ::BOOL bInitialOwner, ::LPCWSTR lpName) noexcept
        {
            if (t_in_hook || !lpName) return hooks::CreateMutexW(lpMutexAttributes, bInitialOwner, lpName);
            t_in_hook = true;
            ::HANDLE result = hooks::CreateMutexW(lpMutexAttributes, bInitialOwner, get_isolated_name(lpName).c_str());
            t_in_hook = false; return result;
        }

        ::HANDLE WINAPI CreateMutexA(::LPSECURITY_ATTRIBUTES lpMutexAttributes, ::BOOL bInitialOwner, ::LPCSTR lpName) noexcept
        {
            if (t_in_hook || !lpName) return hooks::CreateMutexA(lpMutexAttributes, bInitialOwner, lpName);
            t_in_hook = true; std::wstring nameW = windower::to_wstring(std::u8string_view(reinterpret_cast<const char8_t*>(lpName)));
            ::HANDLE result = hooks::CreateMutexW(lpMutexAttributes, bInitialOwner, get_isolated_name(nameW.c_str()).c_str());
            t_in_hook = false; return result;
        }

        ::HANDLE WINAPI OpenMutexW(::DWORD dwDesiredAccess, ::BOOL bInheritHandle, ::LPCWSTR lpName) noexcept
        {
            if (t_in_hook || !lpName) return hooks::OpenMutexW(dwDesiredAccess, bInheritHandle, lpName);
            t_in_hook = true;
            ::HANDLE result = hooks::OpenMutexW(dwDesiredAccess, bInheritHandle, get_isolated_name(lpName).c_str());
            t_in_hook = false; return result;
        }

        ::HANDLE WINAPI OpenMutexA(::DWORD dwDesiredAccess, ::BOOL bInheritHandle, ::LPCSTR lpName) noexcept
        {
            if (t_in_hook || !lpName) return hooks::OpenMutexA(dwDesiredAccess, bInheritHandle, lpName);
            t_in_hook = true; std::wstring nameW = windower::to_wstring(std::u8string_view(reinterpret_cast<const char8_t*>(lpName)));
            ::HANDLE result = hooks::OpenMutexW(dwDesiredAccess, bInheritHandle, get_isolated_name(nameW.c_str()).c_str());
            t_in_hook = false; return result;
        }

        // Events
        ::HANDLE WINAPI CreateEventW(::LPSECURITY_ATTRIBUTES lpEventAttributes, ::BOOL bManualReset, ::BOOL bInitialState, ::LPCWSTR lpName) noexcept
        {
            if (t_in_hook || !lpName) return hooks::CreateEventW(lpEventAttributes, bManualReset, bInitialState, lpName);
            t_in_hook = true;
            ::HANDLE result = hooks::CreateEventW(lpEventAttributes, bManualReset, bInitialState, get_isolated_name(lpName).c_str());
            t_in_hook = false; return result;
        }

        ::HANDLE WINAPI CreateEventA(::LPSECURITY_ATTRIBUTES lpEventAttributes, ::BOOL bManualReset, ::BOOL bInitialState, ::LPCSTR lpName) noexcept
        {
            if (t_in_hook || !lpName) return hooks::CreateEventA(lpEventAttributes, bManualReset, bInitialState, lpName);
            t_in_hook = true; std::wstring nameW = windower::to_wstring(std::u8string_view(reinterpret_cast<const char8_t*>(lpName)));
            ::HANDLE result = hooks::CreateEventW(lpEventAttributes, bManualReset, bInitialState, get_isolated_name(nameW.c_str()).c_str());
            t_in_hook = false; return result;
        }

        ::HANDLE WINAPI OpenEventW(::DWORD dwDesiredAccess, ::BOOL bInheritHandle, ::LPCWSTR lpName) noexcept
        {
            if (t_in_hook || !lpName) return hooks::OpenEventW(dwDesiredAccess, bInheritHandle, lpName);
            t_in_hook = true;
            ::HANDLE result = hooks::OpenEventW(dwDesiredAccess, bInheritHandle, get_isolated_name(lpName).c_str());
            t_in_hook = false; return result;
        }

        ::HANDLE WINAPI OpenEventA(::DWORD dwDesiredAccess, ::BOOL bInheritHandle, ::LPCSTR lpName) noexcept
        {
            if (t_in_hook || !lpName) return hooks::OpenEventA(dwDesiredAccess, bInheritHandle, lpName);
            t_in_hook = true; std::wstring nameW = windower::to_wstring(std::u8string_view(reinterpret_cast<const char8_t*>(lpName)));
            ::HANDLE result = hooks::OpenEventW(dwDesiredAccess, bInheritHandle, get_isolated_name(nameW.c_str()).c_str());
            t_in_hook = false; return result;
        }

        // Semaphores
        ::HANDLE WINAPI CreateSemaphoreW(::LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, ::LONG lInitialCount, ::LONG lMaximumCount, ::LPCWSTR lpName) noexcept
        {
            if (t_in_hook || !lpName) return hooks::CreateSemaphoreW(lpSemaphoreAttributes, lInitialCount, lMaximumCount, lpName);
            t_in_hook = true;
            ::HANDLE result = hooks::CreateSemaphoreW(lpSemaphoreAttributes, lInitialCount, lMaximumCount, get_isolated_name(lpName).c_str());
            t_in_hook = false; return result;
        }

        ::HANDLE WINAPI CreateSemaphoreA(::LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, ::LONG lInitialCount, ::LONG lMaximumCount, ::LPCSTR lpName) noexcept
        {
            if (t_in_hook || !lpName) return hooks::CreateSemaphoreA(lpSemaphoreAttributes, lInitialCount, lMaximumCount, lpName);
            t_in_hook = true; std::wstring nameW = windower::to_wstring(std::u8string_view(reinterpret_cast<const char8_t*>(lpName)));
            ::HANDLE result = hooks::CreateSemaphoreW(lpSemaphoreAttributes, lInitialCount, lMaximumCount, get_isolated_name(nameW.c_str()).c_str());
            t_in_hook = false; return result;
        }

        ::HANDLE WINAPI OpenSemaphoreW(::DWORD dwDesiredAccess, ::BOOL bInheritHandle, ::LPCWSTR lpName) noexcept
        {
            if (t_in_hook || !lpName) return hooks::OpenSemaphoreW(dwDesiredAccess, bInheritHandle, lpName);
            t_in_hook = true;
            ::HANDLE result = hooks::OpenSemaphoreW(dwDesiredAccess, bInheritHandle, get_isolated_name(lpName).c_str());
            t_in_hook = false; return result;
        }

        ::HANDLE WINAPI OpenSemaphoreA(::DWORD dwDesiredAccess, ::BOOL bInheritHandle, ::LPCSTR lpName) noexcept
        {
            if (t_in_hook || !lpName) return hooks::OpenSemaphoreA(dwDesiredAccess, bInheritHandle, lpName);
            t_in_hook = true; std::wstring nameW = windower::to_wstring(std::u8string_view(reinterpret_cast<const char8_t*>(lpName)));
            ::HANDLE result = hooks::OpenSemaphoreW(dwDesiredAccess, bInheritHandle, get_isolated_name(nameW.c_str()).c_str());
            t_in_hook = false; return result;
        }

        // File Mappings
        ::HANDLE WINAPI CreateFileMappingW(::HANDLE hFile, ::LPSECURITY_ATTRIBUTES lpFileMappingAttributes, ::DWORD flProtect, ::DWORD dwMaximumSizeHigh, ::DWORD dwMaximumSizeLow, ::LPCWSTR lpName) noexcept
        {
            if (t_in_hook || !lpName) return hooks::CreateFileMappingW(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName);
            t_in_hook = true;
            ::HANDLE result = hooks::CreateFileMappingW(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, get_isolated_name(lpName).c_str());
            t_in_hook = false; return result;
        }

        ::HANDLE WINAPI CreateFileMappingA(::HANDLE hFile, ::LPSECURITY_ATTRIBUTES lpFileMappingAttributes, ::DWORD flProtect, ::DWORD dwMaximumSizeHigh, ::DWORD dwMaximumSizeLow, ::LPCSTR lpName) noexcept
        {
            if (t_in_hook || !lpName) return hooks::CreateFileMappingA(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName);
            t_in_hook = true; std::wstring nameW = windower::to_wstring(std::u8string_view(reinterpret_cast<const char8_t*>(lpName)));
            ::HANDLE result = hooks::CreateFileMappingW(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, get_isolated_name(nameW.c_str()).c_str());
            t_in_hook = false; return result;
        }

        ::HANDLE WINAPI OpenFileMappingW(::DWORD dwDesiredAccess, ::BOOL bInheritHandle, ::LPCWSTR lpName) noexcept
        {
            if (t_in_hook || !lpName) return hooks::OpenFileMappingW(dwDesiredAccess, bInheritHandle, lpName);
            t_in_hook = true;
            ::HANDLE result = hooks::OpenFileMappingW(dwDesiredAccess, bInheritHandle, get_isolated_name(lpName).c_str());
            t_in_hook = false; return result;
        }

        ::HANDLE WINAPI OpenFileMappingA(::DWORD dwDesiredAccess, ::BOOL bInheritHandle, ::LPCSTR lpName) noexcept
        {
            if (t_in_hook || !lpName) return hooks::OpenFileMappingA(dwDesiredAccess, bInheritHandle, lpName);
            t_in_hook = true; std::wstring nameW = windower::to_wstring(std::u8string_view(reinterpret_cast<const char8_t*>(lpName)));
            ::HANDLE result = hooks::OpenFileMappingW(dwDesiredAccess, bInheritHandle, get_isolated_name(nameW.c_str()).c_str());
            t_in_hook = false; return result;
        }
    }
}

// Proxies
::HANDLE windower::kernel32::CreateMutexA(::LPSECURITY_ATTRIBUTES lpMutexAttributes, ::BOOL bInitialOwner, ::LPCSTR lpName) noexcept { return hooks::CreateMutexA ? hooks::CreateMutexA(lpMutexAttributes, bInitialOwner, lpName) : ::CreateMutexA(lpMutexAttributes, bInitialOwner, lpName); }
::HANDLE windower::kernel32::CreateMutexW(::LPSECURITY_ATTRIBUTES lpMutexAttributes, ::BOOL bInitialOwner, ::LPCWSTR lpName) noexcept { return hooks::CreateMutexW ? hooks::CreateMutexW(lpMutexAttributes, bInitialOwner, lpName) : ::CreateMutexW(lpMutexAttributes, bInitialOwner, lpName); }
::HANDLE windower::kernel32::OpenMutexA(::DWORD dwDesiredAccess, ::BOOL bInheritHandle, ::LPCSTR lpName) noexcept { return hooks::OpenMutexA ? hooks::OpenMutexA(dwDesiredAccess, bInheritHandle, lpName) : ::OpenMutexA(dwDesiredAccess, bInheritHandle, lpName); }
::HANDLE windower::kernel32::OpenMutexW(::DWORD dwDesiredAccess, ::BOOL bInheritHandle, ::LPCWSTR lpName) noexcept { return hooks::OpenMutexW ? hooks::OpenMutexW(dwDesiredAccess, bInheritHandle, lpName) : ::OpenMutexW(dwDesiredAccess, bInheritHandle, lpName); }
::HANDLE windower::kernel32::CreateEventA(::LPSECURITY_ATTRIBUTES lpEventAttributes, ::BOOL bManualReset, ::BOOL bInitialState, ::LPCSTR lpName) noexcept { return hooks::CreateEventA ? hooks::CreateEventA(lpEventAttributes, bManualReset, bInitialState, lpName) : ::CreateEventA(lpEventAttributes, bManualReset, bInitialState, lpName); }
::HANDLE windower::kernel32::CreateEventW(::LPSECURITY_ATTRIBUTES lpEventAttributes, ::BOOL bManualReset, ::BOOL bInitialState, ::LPCWSTR lpName) noexcept { return hooks::CreateEventW ? hooks::CreateEventW(lpEventAttributes, bManualReset, bInitialState, lpName) : ::CreateEventW(lpEventAttributes, bManualReset, bInitialState, lpName); }
::HANDLE windower::kernel32::OpenEventA(::DWORD dwDesiredAccess, ::BOOL bInheritHandle, ::LPCSTR lpName) noexcept { return hooks::OpenEventA ? hooks::OpenEventA(dwDesiredAccess, bInheritHandle, lpName) : ::OpenEventA(dwDesiredAccess, bInheritHandle, lpName); }
::HANDLE windower::kernel32::OpenEventW(::DWORD dwDesiredAccess, ::BOOL bInheritHandle, ::LPCWSTR lpName) noexcept { return hooks::OpenEventW ? hooks::OpenEventW(dwDesiredAccess, bInheritHandle, lpName) : ::OpenEventW(dwDesiredAccess, bInheritHandle, lpName); }

::HANDLE windower::kernel32::CreateSemaphoreA(::LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, ::LONG lInitialCount, ::LONG lMaximumCount, ::LPCSTR lpName) noexcept { return hooks::CreateSemaphoreA ? hooks::CreateSemaphoreA(lpSemaphoreAttributes, lInitialCount, lMaximumCount, lpName) : ::CreateSemaphoreA(lpSemaphoreAttributes, lInitialCount, lMaximumCount, lpName); }
::HANDLE windower::kernel32::CreateSemaphoreW(::LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, ::LONG lInitialCount, ::LONG lMaximumCount, ::LPCWSTR lpName) noexcept { return hooks::CreateSemaphoreW ? hooks::CreateSemaphoreW(lpSemaphoreAttributes, lInitialCount, lMaximumCount, lpName) : ::CreateSemaphoreW(lpSemaphoreAttributes, lInitialCount, lMaximumCount, lpName); }
::HANDLE windower::kernel32::OpenSemaphoreA(::DWORD dwDesiredAccess, ::BOOL bInheritHandle, ::LPCSTR lpName) noexcept { return hooks::OpenSemaphoreA ? hooks::OpenSemaphoreA(dwDesiredAccess, bInheritHandle, lpName) : ::OpenSemaphoreA(dwDesiredAccess, bInheritHandle, lpName); }
::HANDLE windower::kernel32::OpenSemaphoreW(::DWORD dwDesiredAccess, ::BOOL bInheritHandle, ::LPCWSTR lpName) noexcept { return hooks::OpenSemaphoreW ? hooks::OpenSemaphoreW(dwDesiredAccess, bInheritHandle, lpName) : ::OpenSemaphoreW(dwDesiredAccess, bInheritHandle, lpName); }

::HANDLE windower::kernel32::CreateFileMappingA(::HANDLE hFile, ::LPSECURITY_ATTRIBUTES lpFileMappingAttributes, ::DWORD flProtect, ::DWORD dwMaximumSizeHigh, ::DWORD dwMaximumSizeLow, ::LPCSTR lpName) noexcept { return hooks::CreateFileMappingA ? hooks::CreateFileMappingA(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName) : ::CreateFileMappingA(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName); }
::HANDLE windower::kernel32::CreateFileMappingW(::HANDLE hFile, ::LPSECURITY_ATTRIBUTES lpFileMappingAttributes, ::DWORD flProtect, ::DWORD dwMaximumSizeHigh, ::DWORD dwMaximumSizeLow, ::LPCWSTR lpName) noexcept { return hooks::CreateFileMappingW ? hooks::CreateFileMappingW(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName) : ::CreateFileMappingW(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName); }
::HANDLE windower::kernel32::OpenFileMappingA(::DWORD dwDesiredAccess, ::BOOL bInheritHandle, ::LPCSTR lpName) noexcept { return hooks::OpenFileMappingA ? hooks::OpenFileMappingA(dwDesiredAccess, bInheritHandle, lpName) : ::OpenFileMappingA(dwDesiredAccess, bInheritHandle, lpName); }
::HANDLE windower::kernel32::OpenFileMappingW(::DWORD dwDesiredAccess, ::BOOL bInheritHandle, ::LPCWSTR lpName) noexcept { return hooks::OpenFileMappingW ? hooks::OpenFileMappingW(dwDesiredAccess, bInheritHandle, lpName) : ::OpenFileMappingW(dwDesiredAccess, bInheritHandle, lpName); }

void windower::kernel32::install_sync()
{
    if (!hooks::CreateMutexW)
    {
        hooks::CreateMutexA = hooklib::make_hook(u8"kernel32.dll", u8"CreateMutexA", callbacks::CreateMutexA);
        hooks::CreateMutexW = hooklib::make_hook(u8"kernel32.dll", u8"CreateMutexW", callbacks::CreateMutexW);
        hooks::OpenMutexA = hooklib::make_hook(u8"kernel32.dll", u8"OpenMutexA", callbacks::OpenMutexA);
        hooks::OpenMutexW = hooklib::make_hook(u8"kernel32.dll", u8"OpenMutexW", callbacks::OpenMutexW);

        hooks::CreateEventA = hooklib::make_hook(u8"kernel32.dll", u8"CreateEventA", callbacks::CreateEventA);
        hooks::CreateEventW = hooklib::make_hook(u8"kernel32.dll", u8"CreateEventW", callbacks::CreateEventW);
        hooks::OpenEventA = hooklib::make_hook(u8"kernel32.dll", u8"OpenEventA", callbacks::OpenEventA);
        hooks::OpenEventW = hooklib::make_hook(u8"kernel32.dll", u8"OpenEventW", callbacks::OpenEventW);

        hooks::CreateSemaphoreA = hooklib::make_hook(u8"kernel32.dll", u8"CreateSemaphoreA", callbacks::CreateSemaphoreA);
        hooks::CreateSemaphoreW = hooklib::make_hook(u8"kernel32.dll", u8"CreateSemaphoreW", callbacks::CreateSemaphoreW);
        hooks::OpenSemaphoreA = hooklib::make_hook(u8"kernel32.dll", u8"OpenSemaphoreA", callbacks::OpenSemaphoreA);
        hooks::OpenSemaphoreW = hooklib::make_hook(u8"kernel32.dll", u8"OpenSemaphoreW", callbacks::OpenSemaphoreW);

        hooks::CreateFileMappingA = hooklib::make_hook(u8"kernel32.dll", u8"CreateFileMappingA", callbacks::CreateFileMappingA);
        hooks::CreateFileMappingW = hooklib::make_hook(u8"kernel32.dll", u8"CreateFileMappingW", callbacks::CreateFileMappingW);
        hooks::OpenFileMappingA = hooklib::make_hook(u8"kernel32.dll", u8"OpenFileMappingA", callbacks::OpenFileMappingA);
        hooks::OpenFileMappingW = hooklib::make_hook(u8"kernel32.dll", u8"OpenFileMappingW", callbacks::OpenFileMappingW);
    }
}

void windower::kernel32::uninstall_sync() noexcept
{
    hooks::CreateMutexA = {}; hooks::CreateMutexW = {};
    hooks::OpenMutexA = {}; hooks::OpenMutexW = {};
    hooks::CreateEventA = {}; hooks::CreateEventW = {};
    hooks::OpenEventA = {}; hooks::OpenEventW = {};
    hooks::CreateSemaphoreA = {}; hooks::CreateSemaphoreW = {};
    hooks::OpenSemaphoreA = {}; hooks::OpenSemaphoreW = {};
    hooks::CreateFileMappingA = {}; hooks::CreateFileMappingW = {};
    hooks::OpenFileMappingA = {}; hooks::OpenFileMappingW = {};
}
