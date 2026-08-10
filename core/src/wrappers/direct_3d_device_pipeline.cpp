#include "wrappers/direct_3d_device.hpp"
#include "wrappers/direct_3d.hpp"
#include "core.hpp"

// ============================================================================
// GRAPHICS PIPELINE, STATE MANAGEMENT & DRAWING
// ============================================================================

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::TestCooperativeLevel() noexcept
{
    return m_impl->TestCooperativeLevel();
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::GetDeviceCaps(::D3DCAPS8* pCaps) noexcept
{
    return m_impl->GetDeviceCaps(pCaps);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::GetDisplayMode(::D3DDISPLAYMODE* pMode) noexcept
{
    return m_impl->GetDisplayMode(pMode);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetCreationParameters(
    ::D3DDEVICE_CREATION_PARAMETERS* pParameters) noexcept
{
    return m_impl->GetCreationParameters(pParameters);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetCursorProperties(
    ::UINT XHotSpot, ::UINT YHotSpot,
    ::IDirect3DSurface8* pCursorBitmap) noexcept
{
    return m_impl->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

void STDMETHODCALLTYPE windower::direct_3d_device::SetCursorPosition(
    int X, int Y, ::DWORD Flags) noexcept
{
    return m_impl->SetCursorPosition(X, Y, Flags);
}

::BOOL STDMETHODCALLTYPE
windower::direct_3d_device::ShowCursor(::BOOL bShow) noexcept
{
    return m_impl->ShowCursor(bShow);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetRasterStatus(
    ::D3DRASTER_STATUS* pRasterStatus) noexcept
{
    return m_impl->GetRasterStatus(pRasterStatus);
}

void STDMETHODCALLTYPE windower::direct_3d_device::SetGammaRamp(
    ::DWORD Flags, ::D3DGAMMARAMP const* pRamp) noexcept
{
    return m_impl->SetGammaRamp(Flags, pRamp);
}

void STDMETHODCALLTYPE
windower::direct_3d_device::GetGammaRamp(::D3DGAMMARAMP* pRamp) noexcept
{
    return m_impl->GetGammaRamp(pRamp);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetRenderTarget(
    ::IDirect3DSurface8* pRenderTarget,
    ::IDirect3DSurface8* pNewZStencil) noexcept
{
    return m_impl->SetRenderTarget(pRenderTarget, pNewZStencil);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::Clear(
    ::DWORD Count, ::D3DRECT const* pRects, ::DWORD Flags, ::D3DCOLOR Color,
    float Z, ::DWORD Stencil) noexcept
{
    return m_impl->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetTransform(
    ::D3DTRANSFORMSTATETYPE State, ::D3DMATRIX const* pMatrix) noexcept
{
    if (pMatrix)
    {
        if (State == D3DTS_VIEW)
        {
            core::instance().view_matrix = *pMatrix;
        }
        else if (State == D3DTS_PROJECTION)
        {
            core::instance().projection_matrix = *pMatrix;
        }
    }
    return m_impl->SetTransform(State, pMatrix);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetTransform(
    ::D3DTRANSFORMSTATETYPE State, ::D3DMATRIX* pMatrix) noexcept
{
    return m_impl->GetTransform(State, pMatrix);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::MultiplyTransform(
    ::D3DTRANSFORMSTATETYPE State, ::D3DMATRIX const* pMatrix) noexcept
{
    return m_impl->MultiplyTransform(State, pMatrix);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetViewport(
    ::D3DVIEWPORT8 const* pViewport) noexcept
{
    if (pViewport)
    {
        core::instance().viewport = *pViewport;
    }
    return m_impl->SetViewport(pViewport);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::GetViewport(::D3DVIEWPORT8* pViewport) noexcept
{
    return m_impl->GetViewport(pViewport);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetMaterial(
    ::D3DMATERIAL8 const* pMaterial) noexcept
{
    return m_impl->SetMaterial(pMaterial);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::GetMaterial(::D3DMATERIAL8* pMaterial) noexcept
{
    return m_impl->GetMaterial(pMaterial);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetLight(
    ::DWORD Index, ::D3DLIGHT8 const* pLight) noexcept
{
    return m_impl->SetLight(Index, pLight);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetLight(
    ::DWORD Index, ::D3DLIGHT8* pLight) noexcept
{
    return m_impl->GetLight(Index, pLight);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::LightEnable(::DWORD Index, ::BOOL Enable) noexcept
{
    return m_impl->LightEnable(Index, Enable);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetLightEnable(
    ::DWORD Index, ::BOOL* pEnable) noexcept
{
    return m_impl->GetLightEnable(Index, pEnable);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetClipPlane(
    ::DWORD Index, float const* pPlane) noexcept
{
    return m_impl->SetClipPlane(Index, pPlane);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::GetClipPlane(::DWORD Index, float* pPlane) noexcept
{
    return m_impl->GetClipPlane(Index, pPlane);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetRenderState(
    ::D3DRENDERSTATETYPE State, ::DWORD Value) noexcept
{
    return m_impl->SetRenderState(State, Value);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetRenderState(
    ::D3DRENDERSTATETYPE State, ::DWORD* pValue) noexcept
{
    return m_impl->GetRenderState(State, pValue);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::BeginStateBlock() noexcept
{
    return m_impl->BeginStateBlock();
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::EndStateBlock(::DWORD* pToken) noexcept
{
    return m_impl->EndStateBlock(pToken);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::ApplyStateBlock(::DWORD Token) noexcept
{
    return m_impl->ApplyStateBlock(Token);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::CaptureStateBlock(::DWORD Token) noexcept
{
    return m_impl->CaptureStateBlock(Token);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetClipStatus(
    ::D3DCLIPSTATUS8 const* pClipStatus) noexcept
{
    return m_impl->SetClipStatus(pClipStatus);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetClipStatus(
    ::D3DCLIPSTATUS8* pClipStatus) noexcept
{
    return m_impl->GetClipStatus(pClipStatus);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetTexture(
    ::DWORD Stage, ::IDirect3DBaseTexture8** ppTexture) noexcept
{
    return m_impl->GetTexture(Stage, ppTexture);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetTexture(
    ::DWORD Stage, ::IDirect3DBaseTexture8* pTexture) noexcept
{
    return m_impl->SetTexture(Stage, pTexture);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetTextureStageState(
    ::DWORD Stage, ::D3DTEXTURESTAGESTATETYPE Type, ::DWORD* pValue) noexcept
{
    return m_impl->GetTextureStageState(Stage, Type, pValue);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetTextureStageState(
    ::DWORD Stage, ::D3DTEXTURESTAGESTATETYPE Type, ::DWORD Value) noexcept
{
    return m_impl->SetTextureStageState(Stage, Type, Value);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::ValidateDevice(::DWORD* pNumPasses) noexcept
{
    return m_impl->ValidateDevice(pNumPasses);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetInfo(
    ::DWORD DevInfoID, void* pDevInfoStruct, ::DWORD DevInfoStructSize) noexcept
{
    return m_impl->GetInfo(DevInfoID, pDevInfoStruct, DevInfoStructSize);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetPaletteEntries(
    ::UINT PaletteNumber, ::PALETTEENTRY const* pEntries) noexcept
{
    return m_impl->SetPaletteEntries(PaletteNumber, pEntries);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetPaletteEntries(
    ::UINT PaletteNumber, ::PALETTEENTRY* pEntries) noexcept
{
    return m_impl->GetPaletteEntries(PaletteNumber, pEntries);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::SetCurrentTexturePalette(
    ::UINT PaletteNumber) noexcept
{
    return m_impl->SetCurrentTexturePalette(PaletteNumber);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::GetCurrentTexturePalette(
    ::UINT* PaletteNumber) noexcept
{
    return m_impl->GetCurrentTexturePalette(PaletteNumber);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::DrawPrimitive(
    ::D3DPRIMITIVETYPE PrimitiveType, ::UINT StartVertex,
    ::UINT PrimitiveCount) noexcept
{
    return m_impl->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::DrawIndexedPrimitive(
    ::D3DPRIMITIVETYPE PrimitiveType, ::UINT minIndex, ::UINT NumVertices,
    ::UINT startIndex, ::UINT primCount) noexcept
{
    return m_impl->DrawIndexedPrimitive(
        PrimitiveType, minIndex, NumVertices, startIndex, primCount);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::DrawPrimitiveUP(
    ::D3DPRIMITIVETYPE PrimitiveType, ::UINT PrimitiveCount,
    void const* pVertexStreamZeroData, ::UINT VertexStreamZeroStride) noexcept
{
    return m_impl->DrawPrimitiveUP(
        PrimitiveType, PrimitiveCount, pVertexStreamZeroData,
        VertexStreamZeroStride);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::DrawIndexedPrimitiveUP(
    ::D3DPRIMITIVETYPE PrimitiveType, ::UINT MinVertexIndex,
    ::UINT NumVertexIndices, ::UINT PrimitiveCount, void const* pIndexData,
    ::D3DFORMAT IndexDataFormat, void const* pVertexStreamZeroData,
    ::UINT VertexStreamZeroStride) noexcept
{
    return m_impl->DrawIndexedPrimitiveUP(
        PrimitiveType, MinVertexIndex, NumVertexIndices, PrimitiveCount,
        pIndexData, IndexDataFormat, pVertexStreamZeroData,
        VertexStreamZeroStride);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::ProcessVertices(
    ::UINT SrcStartIndex, ::UINT DestIndex, ::UINT VertexCount,
    ::IDirect3DVertexBuffer8* pDestBuffer, ::DWORD Flags) noexcept
{
    return m_impl->ProcessVertices(
        SrcStartIndex, DestIndex, VertexCount, pDestBuffer, Flags);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::SetVertexShader(::DWORD Handle) noexcept
{
    return m_impl->SetVertexShader(Handle);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::GetVertexShader(::DWORD* pHandle) noexcept
{
    return m_impl->GetVertexShader(pHandle);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetVertexShaderConstant(
    ::DWORD Register, void const* pConstantData, ::DWORD ConstantCount) noexcept
{
    return m_impl->SetVertexShaderConstant(
        Register, pConstantData, ConstantCount);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetVertexShaderConstant(
    ::DWORD Register, void* pConstantData, ::DWORD ConstantCount) noexcept
{
    return m_impl->GetVertexShaderConstant(
        Register, pConstantData, ConstantCount);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetStreamSource(
    ::UINT StreamNumber, ::IDirect3DVertexBuffer8* pStreamData,
    ::UINT Stride) noexcept
{
    return m_impl->SetStreamSource(StreamNumber, pStreamData, Stride);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetStreamSource(
    ::UINT StreamNumber, ::IDirect3DVertexBuffer8** ppStreamData,
    ::UINT* pStride) noexcept
{
    return m_impl->GetStreamSource(StreamNumber, ppStreamData, pStride);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetIndices(
    ::IDirect3DIndexBuffer8* pIndexData, ::UINT BaseVertexIndex) noexcept
{
    return m_impl->SetIndices(pIndexData, BaseVertexIndex);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetIndices(
    ::IDirect3DIndexBuffer8** ppIndexData, ::UINT* pBaseVertexIndex) noexcept
{
    return m_impl->GetIndices(ppIndexData, pBaseVertexIndex);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::SetPixelShader(::DWORD Handle) noexcept
{
    return m_impl->SetPixelShader(Handle);
}

::HRESULT STDMETHODCALLTYPE
windower::direct_3d_device::GetPixelShader(::DWORD* pHandle) noexcept
{
    return m_impl->GetPixelShader(pHandle);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::SetPixelShaderConstant(
    ::DWORD Register, void const* pConstantData, ::DWORD ConstantCount) noexcept
{
    return m_impl->SetPixelShaderConstant(
        Register, pConstantData, ConstantCount);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::GetPixelShaderConstant(
    ::DWORD Register, void* pConstantData, ::DWORD ConstantCount) noexcept
{
    return m_impl->GetPixelShaderConstant(
        Register, pConstantData, ConstantCount);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::DrawRectPatch(
    ::UINT Handle, float const* pNumSegs,
    ::D3DRECTPATCH_INFO const* pRectPatchInfo) noexcept
{
    return m_impl->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::DrawTriPatch(
    ::UINT Handle, float const* pNumSegs,
    ::D3DTRIPATCH_INFO const* pTriPatchInfo) noexcept
{
    return m_impl->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}
