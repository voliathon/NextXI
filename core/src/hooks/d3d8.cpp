#include "hooks/d3d8.hpp"

#include "hooklib/hook.hpp"
#include "wrappers/direct_3d.hpp"

#include <windows.h>

#include <d3d8.h>

#include <memory>

namespace
{

namespace hooks
{

windower::hooklib::hook<decltype(::Direct3DCreate8)> Direct3DCreate8;

}

namespace callbacks
{

::IDirect3D8* WINAPI Direct3DCreate8(::UINT SDKVersion) noexcept
{
    using namespace windower;

    ::MemoryBarrier();
    if (auto ptr = hooks::Direct3DCreate8(SDKVersion))
    {
        return std::make_unique<direct_3d>(ptr).release();
    }
    return nullptr;
}

}

}

::IDirect3D8* windower::d3d8::Direct3DCreate8(::UINT SDKVersion) noexcept
{
    return hooks::Direct3DCreate8(SDKVersion);
}

void windower::d3d8::install()
{
    if (!hooks::Direct3DCreate8)
    {
        // Explicitly force Windows to map the DLL using standard search order.
        // This guarantees that if the launcher placed dgVoodoo2 in the game directory,
        // Windows will lock onto the local proxy DLL instead of the System32 default.
        ::LoadLibraryW(L"d3d8.dll");

        hooks::Direct3DCreate8 = hooklib::make_hook<false>(
            u8"d3d8.dll", u8"Direct3DCreate8", callbacks::Direct3DCreate8);
    }
}

void windower::d3d8::uninstall() noexcept
{
    hooks::Direct3DCreate8 = {};
}
