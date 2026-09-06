#include "wrappers/direct_3d_device.hpp"
#include "wrappers/direct_3d.hpp"

// ============================================================================
// RESOURCE MANAGEMENT & MEMORY ALLOCATION
// ============================================================================

::UINT STDMETHODCALLTYPE
windower::direct_3d_device::GetAvailableTextureMem() noexcept
{
    return m_impl->GetAvailableTextureMem();
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::ResourceManagerDiscardBytes(::DWORD Bytes) noexcept
{
    return m_impl->ResourceManagerDiscardBytes(Bytes);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::GetDirect3D(::IDirect3D8** ppD3D8) noexcept
{
    if (ppD3D8)
    {
        m_parent->AddRef();
        *ppD3D8 = m_parent;
        return S_OK;
    }
    return D3DERR_INVALIDCALL;
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::CreateAdditionalSwapChain(
    ::D3DPRESENT_PARAMETERS* pPresentationParameters,
    ::IDirect3DSwapChain8** pSwapChain) noexcept
{
    return m_impl->CreateAdditionalSwapChain(
        pPresentationParameters, pSwapChain);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetBackBuffer(
    ::UINT BackBuffer, ::D3DBACKBUFFER_TYPE Type,
    ::IDirect3DSurface8** ppBackBuffer) noexcept
{
    return m_impl->GetBackBuffer(BackBuffer, Type, ppBackBuffer);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetFrontBuffer(
    ::IDirect3DSurface8* pDestSurface) noexcept
{
    return m_impl->GetFrontBuffer(pDestSurface);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CreateTexture(
    ::UINT Width, ::UINT Height, ::UINT Levels, ::DWORD Usage,
    ::D3DFORMAT Format, ::D3DPOOL Pool,
    ::IDirect3DTexture8** ppTexture) noexcept
{
    return m_impl->CreateTexture(
        Width, Height, Levels, Usage, Format, Pool, ppTexture);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CreateVolumeTexture(
    ::UINT Width, ::UINT Height, ::UINT Depth, ::UINT Levels, ::DWORD Usage,
    ::D3DFORMAT Format, ::D3DPOOL Pool,
    ::IDirect3DVolumeTexture8** ppVolumeTexture) noexcept
{
    return m_impl->CreateVolumeTexture(
        Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CreateCubeTexture(
    ::UINT EdgeLength, ::UINT Levels, ::DWORD Usage, ::D3DFORMAT Format,
    ::D3DPOOL Pool, ::IDirect3DCubeTexture8** ppCubeTexture) noexcept
{
    return m_impl->CreateCubeTexture(
        EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CreateVertexBuffer(
    ::UINT Length, ::DWORD Usage, ::DWORD FVF, ::D3DPOOL Pool,
    ::IDirect3DVertexBuffer8** ppVertexBuffer) noexcept
{
    return m_impl->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CreateIndexBuffer(
    ::UINT Length, ::DWORD Usage, ::D3DFORMAT Format, ::D3DPOOL Pool,
    ::IDirect3DIndexBuffer8** ppIndexBuffer) noexcept
{
    return m_impl->CreateIndexBuffer(
        Length, Usage, Format, Pool, ppIndexBuffer);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CreateRenderTarget(
    ::UINT Width, ::UINT Height, ::D3DFORMAT Format,
    ::D3DMULTISAMPLE_TYPE MultiSample, ::BOOL Lockable,
    ::IDirect3DSurface8** ppSurface) noexcept
{
    return m_impl->CreateRenderTarget(
        Width, Height, Format, MultiSample, Lockable, ppSurface);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::CreateDepthStencilSurface(
    ::UINT Width, ::UINT Height, ::D3DFORMAT Format,
    ::D3DMULTISAMPLE_TYPE MultiSample, ::IDirect3DSurface8** ppSurface) noexcept
{
    return m_impl->CreateDepthStencilSurface(
        Width, Height, Format, MultiSample, ppSurface);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CreateImageSurface(
    ::UINT Width, ::UINT Height, ::D3DFORMAT Format,
    ::IDirect3DSurface8** ppSurface) noexcept
{
    return m_impl->CreateImageSurface(Width, Height, Format, ppSurface);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CopyRects(
    ::IDirect3DSurface8* pSourceSurface, ::RECT const* pSourceRectsArray,
    ::UINT cRects, ::IDirect3DSurface8* pDestinationSurface,
    ::POINT const* pDestPointsArray) noexcept
{
    return m_impl->CopyRects(
        pSourceSurface, pSourceRectsArray, cRects, pDestinationSurface,
        pDestPointsArray);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::UpdateTexture(
    ::IDirect3DBaseTexture8* pSourceTexture,
    ::IDirect3DBaseTexture8* pDestinationTexture) noexcept
{
    return m_impl->UpdateTexture(pSourceTexture, pDestinationTexture);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetRenderTarget(
    ::IDirect3DSurface8** ppRenderTarget) noexcept
{
    return m_impl->GetRenderTarget(ppRenderTarget);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetDepthStencilSurface(
    ::IDirect3DSurface8** ppZStencilSurface) noexcept
{
    return m_impl->GetDepthStencilSurface(ppZStencilSurface);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CreateStateBlock(
    ::D3DSTATEBLOCKTYPE Type, ::DWORD* pToken) noexcept
{
    return m_impl->CreateStateBlock(Type, pToken);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::DeleteStateBlock(::DWORD Token) noexcept
{
    return m_impl->DeleteStateBlock(Token);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CreateVertexShader(
    ::DWORD const* pDeclaration, ::DWORD const* pFunction, ::DWORD* pHandle,
    ::DWORD Usage) noexcept
{
    return m_impl->CreateVertexShader(pDeclaration, pFunction, pHandle, Usage);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::DeleteVertexShader(::DWORD Handle) noexcept
{
    return m_impl->DeleteVertexShader(Handle);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::GetVertexShaderDeclaration(
    ::DWORD Handle, void* pData, ::DWORD* pSizeOfData) noexcept
{
    return m_impl->GetVertexShaderDeclaration(Handle, pData, pSizeOfData);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetVertexShaderFunction(
    ::DWORD Handle, void* pData, ::DWORD* pSizeOfData) noexcept
{
    return m_impl->GetVertexShaderFunction(Handle, pData, pSizeOfData);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::CreatePixelShader(
    ::DWORD const* pFunction, ::DWORD* pHandle) noexcept
{
    return m_impl->CreatePixelShader(pFunction, pHandle);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::DeletePixelShader(::DWORD Handle) noexcept
{
    return m_impl->DeletePixelShader(Handle);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetPixelShaderFunction(
    ::DWORD Handle, void* pData, ::DWORD* pSizeOfData) noexcept
{
    return m_impl->GetPixelShaderFunction(Handle, pData, pSizeOfData);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::DeletePatch(::UINT Handle) noexcept
{
    return m_impl->DeletePatch(Handle);
}
