#include "hooks/kernel32.hpp"
#include "kernel32_internal.hpp"
#include "hooklib/hook.hpp"
#include "core.hpp"
#include "unicode.hpp"
#include "utilities/paths.hpp"

#include <windows.h>
#include <algorithm>

namespace
{
    ::DWORD priority = THREAD_PRIORITY_NORMAL;

    namespace hooks
    {
        windower::hooklib::hook<decltype(::GetPriorityClass)> GetPriorityClass;
        windower::hooklib::hook<decltype(::SetPriorityClass)> SetPriorityClass;
        windower::hooklib::hook<decltype(::CreateProcessW)> CreateProcessW;
    }

    namespace callbacks
    {
        ::DWORD WINAPI GetPriorityClass(::HANDLE hProcess) noexcept
        {
            if (::GetProcessId(hProcess) == ::GetCurrentProcessId()) return priority;
            return hooks::GetPriorityClass(hProcess);
        }

        ::BOOL WINAPI SetPriorityClass(::HANDLE hProcess, ::DWORD dwPriorityClass) noexcept
        {
            if (::GetProcessId(hProcess) == ::GetCurrentProcessId())
            {
                ::InterlockedExchange(&priority, dwPriorityClass);
                return TRUE;
            }
            return hooks::SetPriorityClass(hProcess, dwPriorityClass);
        }

        ::BOOL WINAPI CreateProcessW(
            ::LPCWSTR lpApplicationName, ::LPWSTR lpCommandLine,
            ::LPSECURITY_ATTRIBUTES lpProcessAttributes, ::LPSECURITY_ATTRIBUTES lpThreadAttributes,
            ::BOOL bInheritHandles, ::DWORD dwCreationFlags, ::LPVOID lpEnvironment,
            ::LPCWSTR lpCurrentDirectory, ::LPSTARTUPINFOW lpStartupInfo,
            ::LPPROCESS_INFORMATION lpProcessInformation) noexcept
        {
            if (lpApplicationName && lpCommandLine)
            {
                auto name = std::max(std::wcsrchr(lpApplicationName, '\\'), std::wcsrchr(lpApplicationName, '/'));
                name = name ? std::next(name, 1) : lpApplicationName;
                if (_wcsicmp(name, L"pol.exe") == 0)
                {
                    STARTUPINFO startup{};
                    startup.cb = sizeof startup;

                    auto path = windower::windower_path() / u8"NextXI.exe";
                    auto u8_args = u8R"(")" + path.u8string() + u8R"(" )" + windower::core::instance().settings.command_line_args;
                    auto args = windower::to_wstring(u8_args);

                    return hooks::CreateProcessW(path.c_str(), args.data(), nullptr, nullptr, false, 0, nullptr, nullptr, &startup, lpProcessInformation);
                }
            }
            return hooks::CreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
        }
    }
}

::DWORD windower::kernel32::GetPriorityClass(::HANDLE hProcess) noexcept
{
    return hooks::GetPriorityClass ? hooks::GetPriorityClass(hProcess) : ::GetPriorityClass(hProcess);
}

::BOOL windower::kernel32::SetPriorityClass(::HANDLE hProcess, ::DWORD dwPriorityClass) noexcept
{
    return hooks::SetPriorityClass ? hooks::SetPriorityClass(hProcess, dwPriorityClass) : ::SetPriorityClass(hProcess, dwPriorityClass);
}

::BOOL windower::kernel32::CreateProcessA(
    ::LPCSTR lpApplicationName, ::LPSTR lpCommandLine, ::LPSECURITY_ATTRIBUTES lpProcessAttributes,
    ::LPSECURITY_ATTRIBUTES lpThreadAttributes, ::BOOL bInheritHandles, ::DWORD dwCreationFlags,
    ::LPVOID lpEnvironment, ::LPCSTR lpCurrentDirectory, ::LPSTARTUPINFOA lpStartupInfo, ::LPPROCESS_INFORMATION lpProcessInformation) noexcept
{
    return ::CreateProcessA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

::BOOL windower::kernel32::CreateProcessW(
    ::LPCWSTR lpApplicationName, ::LPWSTR lpCommandLine, ::LPSECURITY_ATTRIBUTES lpProcessAttributes,
    ::LPSECURITY_ATTRIBUTES lpThreadAttributes, ::BOOL bInheritHandles, ::DWORD dwCreationFlags,
    ::LPVOID lpEnvironment, ::LPCWSTR lpCurrentDirectory, ::LPSTARTUPINFOW lpStartupInfo, ::LPPROCESS_INFORMATION lpProcessInformation) noexcept
{
    return hooks::CreateProcessW ? hooks::CreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation)
        : ::CreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

void windower::kernel32::install_process()
{
    if (!hooks::GetPriorityClass)
    {
        priority = ::GetPriorityClass(::GetCurrentProcess());
        hooks::GetPriorityClass = hooklib::make_hook(u8"kernel32.dll", u8"GetPriorityClass", callbacks::GetPriorityClass);
        hooks::SetPriorityClass = hooklib::make_hook(u8"kernel32.dll", u8"SetPriorityClass", callbacks::SetPriorityClass);
        if (!::IsDebuggerPresent())
        {
            hooks::CreateProcessW = hooklib::make_hook(u8"kernel32.dll", u8"CreateProcessW", callbacks::CreateProcessW);
        }
    }
}

void windower::kernel32::uninstall_process() noexcept
{
    hooks::GetPriorityClass = {};
    hooks::SetPriorityClass = {};
    hooks::CreateProcessW = {};
}
