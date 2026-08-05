#ifndef WINDOWER_WRAPPERS_DIRECT_3D_HPP
#define WINDOWER_WRAPPERS_DIRECT_3D_HPP

#include <windows.h>
#include <d3d8.h>

namespace windower
{

class direct_3d final : public ::IDirect3D8
{
public:
    direct_3d() = delete;
    direct_3d(::IDirect3D8*) noexcept;
    direct_3d(direct_3d const&) = delete;
    direct_3d(direct_3d&&) = delete;
    virtual ~direct_3d();

    direct_3d& operator=(direct_3d const&) = delete;
    direct_3d& operator=(direct_3d&&) = delete;
    ::HRESULT STDMETHODCALLTYPE
    QueryInterface(::IID const&, void**) noexcept override;
    ::ULONG STDMETHODCALLTYPE AddRef() noexcept override;
    ::ULONG STDMETHODCALLTYPE Release() noexcept override;
    ::HRESULT STDMETHODCALLTYPE RegisterSoftwareDevice(void*) noexcept override;
    ::UINT STDMETHODCALLTYPE GetAdapterCount() noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetAdapterIdentifier(
        ::UINT, ::DWORD, ::D3DADAPTER_IDENTIFIER8*) noexcept override;
    ::UINT STDMETHODCALLTYPE GetAdapterModeCount(::UINT) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    EnumAdapterModes(::UINT, ::UINT, ::D3DDISPLAYMODE*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetAdapterDisplayMode(::UINT, ::D3DDISPLAYMODE*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CheckDeviceType(
        ::UINT, ::D3DDEVTYPE, ::D3DFORMAT, ::D3DFORMAT,
        ::BOOL) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CheckDeviceFormat(
        ::UINT, ::D3DDEVTYPE, ::D3DFORMAT, ::DWORD, ::D3DRESOURCETYPE,
        ::D3DFORMAT) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CheckDeviceMultiSampleType(
        ::UINT, ::D3DDEVTYPE, ::D3DFORMAT, ::BOOL,
        ::D3DMULTISAMPLE_TYPE) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CheckDepthStencilMatch(
        ::UINT, ::D3DDEVTYPE, ::D3DFORMAT, ::D3DFORMAT,
        ::D3DFORMAT) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetDeviceCaps(::UINT, ::D3DDEVTYPE, ::D3DCAPS8*) noexcept override;
    ::HMONITOR STDMETHODCALLTYPE GetAdapterMonitor(::UINT) noexcept override;
    ::HRESULT STDMETHODCALLTYPE CreateDevice(
        ::UINT, ::D3DDEVTYPE, ::HWND, ::DWORD, ::D3DPRESENT_PARAMETERS*,
        ::IDirect3DDevice8**) noexcept override;

private:
    ::IDirect3D8* m_impl;
    ::ULONG m_count = 0;
};

}

#endif