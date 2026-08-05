#ifndef WINDOWER_HOOKS_DINPUT8_HPP
#define WINDOWER_HOOKS_DINPUT8_HPP

#include <windows.h>
#include <dinput.h>

namespace windower::dinput8
{

::HRESULT DirectInput8Create(
    ::HINSTANCE, ::DWORD, ::IID const&, ::LPVOID*, ::LPUNKNOWN) noexcept;

void install();
void uninstall() noexcept;

}

#endif