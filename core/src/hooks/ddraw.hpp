#ifndef WINDOWER_HOOKS_DDRAW_HPP
#define WINDOWER_HOOKS_DDRAW_HPP

#include <windows.h>

#include <ddraw.h>

namespace windower::ddraw
{

::HRESULT
DirectDrawCreateEx(::GUID*, ::LPVOID*, ::IID const&, ::IUnknown*) noexcept;

void install();
void uninstall() noexcept;

}

#endif