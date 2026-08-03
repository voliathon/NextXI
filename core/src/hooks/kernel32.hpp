#ifndef WINDOWER_HOOKS_KERNEL32_HPP
#define WINDOWER_HOOKS_KERNEL32_HPP

#include <windows.h>

namespace windower::kernel32
{

::UINT GetACP() noexcept;
::HMODULE LoadLibraryA(::LPCSTR);
::HMODULE LoadLibraryW(::LPCWSTR);
::HANDLE CreateMutexA(::LPSECURITY_ATTRIBUTES, ::BOOL, ::LPCSTR) noexcept;
::HANDLE CreateMutexW(::LPSECURITY_ATTRIBUTES, ::BOOL, ::LPCWSTR) noexcept;
::HANDLE OpenMutexA(::DWORD, ::BOOL, ::LPCSTR) noexcept;
::HANDLE OpenMutexW(::DWORD, ::BOOL, ::LPCWSTR) noexcept;
::DWORD GetPriorityClass(::HANDLE) noexcept;
::BOOL SetPriorityClass(::HANDLE, ::DWORD) noexcept;
::BOOL CreateProcessA(
    ::LPCSTR, ::LPSTR, ::LPSECURITY_ATTRIBUTES, ::LPSECURITY_ATTRIBUTES, ::BOOL,
    ::DWORD, ::PVOID, ::LPCSTR, ::LPSTARTUPINFOA,
    ::LPPROCESS_INFORMATION) noexcept;
::BOOL CreateProcessW(
    ::LPCWSTR, ::LPWSTR, ::LPSECURITY_ATTRIBUTES, ::LPSECURITY_ATTRIBUTES,
    ::BOOL, ::DWORD, ::PVOID, ::LPCWSTR, ::LPSTARTUPINFOW,
    ::LPPROCESS_INFORMATION) noexcept;
::LPTOP_LEVEL_EXCEPTION_FILTER SetUnhandledExceptionFilter(
    ::LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) noexcept;

void install();
void uninstall() noexcept;

}

#endif