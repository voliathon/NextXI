#include "hooks/ddraw.hpp"

#include "hooklib/hook.hpp"
#include "wrappers/direct_draw.hpp"

#include <windows.h>

#include <ddraw.h>
#include <winrt/base.h>

#include <memory>

namespace
{

namespace hooks
{

windower::hooklib::hook<decltype(::DirectDrawCreateEx)> DirectDrawCreateEx;

}

namespace callbacks
{

::HRESULT WINAPI DirectDrawCreateEx(
    ::GUID* lpGUID, ::LPVOID* lplpDD, ::IID const& iid,
    ::IUnknown* pUnkOuter) noexcept
{
    using namespace windower;

    if (!lplpDD)
    {
        return E_POINTER;
    }
    *lplpDD = nullptr;

    ::MemoryBarrier();
    winrt::com_ptr<::IDirectDraw7> ptr;
    auto const result =
        hooks::DirectDrawCreateEx(lpGUID, ptr.put_void(), iid, pUnkOuter);
    if (result == S_OK)
    {
        *lplpDD = std::make_unique<direct_draw>(ptr.detach()).release();
    }
    return result;
}

}

}

::HRESULT windower::ddraw::DirectDrawCreateEx(
    ::GUID* lpGUID, ::LPVOID* lplpDD, ::IID const& iid,
    ::IUnknown* pUnkOuter) noexcept
{
    return hooks::DirectDrawCreateEx(lpGUID, lplpDD, iid, pUnkOuter);
}

void windower::ddraw::install()
{
    if (!hooks::DirectDrawCreateEx)
    {
        hooks::DirectDrawCreateEx = hooklib::make_hook<false>(
            u8"ddraw.dll", u8"DirectDrawCreateEx",
            callbacks::DirectDrawCreateEx);
    }
}

void windower::ddraw::uninstall() noexcept { hooks::DirectDrawCreateEx = {}; }