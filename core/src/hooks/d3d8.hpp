#ifndef WINDOWER_HOOKS_D3D8_HPP
#define WINDOWER_HOOKS_D3D8_HPP

#include <windows.h>
#include <d3d8.h>

namespace windower::d3d8
{

::IDirect3D8* Direct3DCreate8(::UINT) noexcept;

void install();
void uninstall() noexcept;

}

#endif