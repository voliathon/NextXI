#ifndef WINDOWER_HOOKS_ADVAPI32_HPP
#define WINDOWER_HOOKS_ADVAPI32_HPP

#include <windows.h>

namespace windower::advapi32
{

::LSTATUS RegEnumValueA(
    ::HKEY, ::DWORD, ::LPSTR, ::LPDWORD, ::LPDWORD, ::LPDWORD, ::LPBYTE,
    ::LPDWORD) noexcept;
::LSTATUS RegEnumValueW(
    ::HKEY, ::DWORD, ::LPWSTR, ::LPDWORD, ::LPDWORD, ::LPDWORD, ::LPBYTE,
    ::LPDWORD) noexcept;
::LSTATUS RegGetValueA(
    ::HKEY, ::LPCSTR, ::LPCSTR, ::DWORD, ::LPDWORD, ::PVOID,
    ::LPDWORD) noexcept;
::LSTATUS RegGetValueW(
    ::HKEY, ::LPCWSTR, ::LPCWSTR, ::DWORD, ::LPDWORD, ::PVOID,
    ::LPDWORD) noexcept;
::LSTATUS RegQueryValueExA(
    ::HKEY, ::LPCSTR, ::LPDWORD, ::LPDWORD, ::LPBYTE, ::LPDWORD) noexcept;
::LSTATUS RegQueryValueExW(
    ::HKEY, ::LPCWSTR, ::LPDWORD, ::LPDWORD, ::LPBYTE, ::LPDWORD) noexcept;

void install();
void uninstall() noexcept;

}

#endif