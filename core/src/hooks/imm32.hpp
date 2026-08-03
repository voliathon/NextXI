#ifndef WINDOWER_HOOKS_IMM32_HPP
#define WINDOWER_HOOKS_IMM32_HPP

#include <windows.h>

namespace windower::imm32
{

::HIMC ImmAssociateContext(::HWND, ::HIMC) noexcept;
::BOOL ImmAssociateContextEx(::HWND, ::HIMC, ::DWORD) noexcept;
::DWORD
    ImmGetCandidateListA(::HIMC, ::DWORD, ::LPCANDIDATELIST, ::DWORD) noexcept;
::LONG ImmGetCompositionStringA(::HIMC, ::DWORD, ::LPVOID, ::DWORD) noexcept;

void install();
void uninstall() noexcept;

}

#endif