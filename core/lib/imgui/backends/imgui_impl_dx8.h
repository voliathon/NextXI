#pragma once
#include "imgui.h"

struct IDirect3DDevice8;
struct ImDrawData;

IMGUI_IMPL_API bool ImGui_ImplDX8_Init(IDirect3DDevice8* device);
IMGUI_IMPL_API void ImGui_ImplDX8_Shutdown();
IMGUI_IMPL_API void ImGui_ImplDX8_NewFrame();
IMGUI_IMPL_API void ImGui_ImplDX8_RenderDrawData(ImDrawData* draw_data);
