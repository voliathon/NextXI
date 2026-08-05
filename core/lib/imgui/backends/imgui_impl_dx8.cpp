#include "imgui.h"
#include "imgui_impl_dx8.h"
#include <d3d8.h>
#include <vector>
#include <cstring>

struct ImGui_ImplDX8_Data {
    IDirect3DDevice8* pd3dDevice;
    IDirect3DTexture8* pFontTexture;
};

static ImGui_ImplDX8_Data* ImGui_ImplDX8_GetBackendData() {
    return ImGui::GetCurrentContext() ? (ImGui_ImplDX8_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;
}

struct CUSTOMVERTEX {
    float x, y, z, rhw;
    D3DCOLOR col;
    float u, v;
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

bool ImGui_ImplDX8_Init(IDirect3DDevice8* device) {
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplDX8_Data* bd = new ImGui_ImplDX8_Data();
    io.BackendRendererUserData = (void*)bd;
    io.BackendRendererName = "imgui_impl_dx8";
    bd->pd3dDevice = device;
    bd->pFontTexture = nullptr;
    return true;
}

void ImGui_ImplDX8_Shutdown() {
    ImGui_ImplDX8_Data* bd = ImGui_ImplDX8_GetBackendData();
    if (!bd) return;
    if (bd->pFontTexture) { bd->pFontTexture->Release(); bd->pFontTexture = nullptr; }
    delete bd;
    ImGui::GetIO().BackendRendererUserData = nullptr;
}

void ImGui_ImplDX8_NewFrame() {
    ImGui_ImplDX8_Data* bd = ImGui_ImplDX8_GetBackendData();
    if (!bd->pFontTexture) {
        ImGuiIO& io = ImGui::GetIO();
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        if (SUCCEEDED(bd->pd3dDevice->CreateTexture(width, height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &bd->pFontTexture))) {
            D3DLOCKED_RECT rect;
            if (SUCCEEDED(bd->pFontTexture->LockRect(0, &rect, nullptr, 0))) {
                for (int y = 0; y < height; y++) {
                    memcpy((unsigned char*)rect.pBits + y * rect.Pitch, pixels + y * (width * 4), width * 4);
                }
                bd->pFontTexture->UnlockRect(0);
            }
        }
        io.Fonts->SetTexID((ImTextureID)bd->pFontTexture);
    }
}

void ImGui_ImplDX8_RenderDrawData(ImDrawData* draw_data) {
    ImGui_ImplDX8_Data* bd = ImGui_ImplDX8_GetBackendData();
    IDirect3DDevice8* device = bd->pd3dDevice;

    // 1. BACKUP FFXI RENDER STATES
    D3DVIEWPORT8 old_vp;
    device->GetViewport(&old_vp);

    DWORD old_z, old_cull, old_alpha, old_src, old_dst, old_fog, old_light, old_atest, old_ps;
    device->GetRenderState(D3DRS_ZENABLE, &old_z);
    device->GetRenderState(D3DRS_CULLMODE, &old_cull);
    device->GetRenderState(D3DRS_ALPHABLENDENABLE, &old_alpha);
    device->GetRenderState(D3DRS_SRCBLEND, &old_src);
    device->GetRenderState(D3DRS_DESTBLEND, &old_dst);
    device->GetRenderState(D3DRS_FOGENABLE, &old_fog);
    device->GetRenderState(D3DRS_LIGHTING, &old_light);
    device->GetRenderState(D3DRS_ALPHATESTENABLE, &old_atest);
    device->GetPixelShader(&old_ps);

    // 2. FORCE IMGUI RENDER STATES (Kill FFXI's shaders and fog)
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    device->SetRenderState(D3DRS_ZENABLE, FALSE);
    device->SetRenderState(D3DRS_FOGENABLE, FALSE);
    device->SetRenderState(D3DRS_LIGHTING, FALSE);
    device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    device->SetPixelShader(0); // KILL ROGUE PIXEL SHADERS

    // 3. FORCE FIXED-FUNCTION TEXTURE STAGES
    device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

    // KILL FFXI'S SECONDARY TEXTURES OR IMGUI TURNS INVISIBLE
    device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

    device->SetVertexShader(D3DFVF_CUSTOMVERTEX);

    std::vector<CUSTOMVERTEX> vertices;
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        vertices.resize(cmd_list->VtxBuffer.Size);
        for (int i = 0; i < cmd_list->VtxBuffer.Size; i++) {
            vertices[i].x = cmd_list->VtxBuffer.Data[i].pos.x;
            vertices[i].y = cmd_list->VtxBuffer.Data[i].pos.y;
            vertices[i].z = 0.0f;
            vertices[i].rhw = 1.0f;
            // D3DCOLOR requires ARGB, but ImGui uses ABGR depending on config. 
            // We pass it directly; if colors look reversed (blue text), we will swap bytes later.
            vertices[i].col = cmd_list->VtxBuffer.Data[i].col;
            vertices[i].u = cmd_list->VtxBuffer.Data[i].uv.x;
            vertices[i].v = cmd_list->VtxBuffer.Data[i].uv.y;
        }

        int idx_offset = 0;
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer.Data[cmd_i];
            if (pcmd->UserCallback) {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else {
                device->SetTexture(0, (IDirect3DBaseTexture8*)pcmd->GetTexID());

                DWORD w = (DWORD)(pcmd->ClipRect.z - pcmd->ClipRect.x);
                DWORD h = (DWORD)(pcmd->ClipRect.w - pcmd->ClipRect.y);
                if (w > 0 && h > 0) {
                    D3DVIEWPORT8 vp;
                    vp.X = (DWORD)pcmd->ClipRect.x;
                    vp.Y = (DWORD)pcmd->ClipRect.y;
                    vp.Width = w;
                    vp.Height = h;
                    vp.MinZ = 0.0f;
                    vp.MaxZ = 1.0f;
                    device->SetViewport(&vp);

                    device->DrawIndexedPrimitiveUP(
                        D3DPT_TRIANGLELIST, 0, cmd_list->VtxBuffer.Size,
                        pcmd->ElemCount / 3, &cmd_list->IdxBuffer.Data[idx_offset],
                        sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32,
                        vertices.data(), sizeof(CUSTOMVERTEX)
                    );
                }
            }
            idx_offset += pcmd->ElemCount;
        }
    }

    // 4. RESTORE FFXI RENDER STATES
    device->SetViewport(&old_vp);
    device->SetRenderState(D3DRS_ZENABLE, old_z);
    device->SetRenderState(D3DRS_CULLMODE, old_cull);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, old_alpha);
    device->SetRenderState(D3DRS_SRCBLEND, old_src);
    device->SetRenderState(D3DRS_DESTBLEND, old_dst);
    device->SetRenderState(D3DRS_FOGENABLE, old_fog);
    device->SetRenderState(D3DRS_LIGHTING, old_light);
    device->SetRenderState(D3DRS_ALPHATESTENABLE, old_atest);
    device->SetPixelShader(old_ps);
}
