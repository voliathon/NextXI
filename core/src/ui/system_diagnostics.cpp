#include "ui/system_diagnostics.hpp"

#include <imgui.h>
#include <windows.h>
#include <string>
#include <vector>
#include <array>

#if __has_include(<luajit.h>)
#include <luajit.h>
#elif __has_include(<lua.h>)
#include <lua.h>
#endif

#pragma comment(lib, "version.lib")

namespace
{
    std::string get_dgvoodoo_version()
    {
        constexpr char const* voodoo_dx = "dgVoodooDirectX.dll";
        constexpr char const* voodoo_std = "dgVoodoo.dll";

        HMODULE hMod = ::GetModuleHandleA(voodoo_dx);
        if (!hMod) hMod = ::GetModuleHandleA(voodoo_std);
        if (!hMod) return "";

        // Safe array to prevent C26485 decay warnings
        std::array<char, MAX_PATH> path_buf{};
        if (::GetModuleFileNameA(hMod, path_buf.data(), MAX_PATH) == 0) return "";

        DWORD dummy = 0;
        DWORD const size = ::GetFileVersionInfoSizeA(path_buf.data(), &dummy);
        if (size == 0) return "";

        std::vector<BYTE> buffer(size);
        if (!::GetFileVersionInfoA(path_buf.data(), 0, size, buffer.data())) return "";

        VS_FIXEDFILEINFO* file_info = nullptr;
        UINT file_info_len = 0;
        if (::VerQueryValueA(buffer.data(), "\\", reinterpret_cast<LPVOID*>(&file_info), &file_info_len))
        {
            auto const ms = file_info->dwFileVersionMS;
            auto const ls = file_info->dwFileVersionLS;
            return " v" + std::to_string((ms >> 16) & 0xFFFF) + "." +
                std::to_string(ms & 0xFFFF) + "." + std::to_string((ls >> 16) & 0xFFFF);
        }
        return "";
    }
}

void windower::ui::system_diagnostics::render_about_tab()
{
    if (ImGui::BeginTabItem("About"))
    {
        ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "NextXI Engine Architecture");
        ImGui::Separator();
        ImGui::Spacing();

        std::string graphics_engine = "Direct3D 8 Proxy";
        if (::GetModuleHandleA("dgVoodooDirectX.dll") != nullptr || ::GetModuleHandleA("dgVoodoo.dll") != nullptr) {
            graphics_engine = "NextXI (dgVoodoo2 / DX11)" + get_dgvoodoo_version();
        }
        else if (::GetModuleHandleA("d3d11.dll") != nullptr) {
            graphics_engine = "Direct3D 11 Proxy";
        }

        std::string lua_version_str = "LuaJIT (Sandbox)";
#if defined(LUAJIT_VERSION)
        lua_version_str = std::string(LUAJIT_VERSION) + " (Sandbox)";
#elif defined(LUA_RELEASE)
        lua_version_str = std::string(LUA_RELEASE) + " (Sandbox)";
#endif

        ImGui::Text("Core Engine Build Date : %s %s", __DATE__, __TIME__);
        ImGui::Text("Graphics Renderer      : %s", graphics_engine.c_str());
        ImGui::Text("Scripting Environment  : %s", lua_version_str.c_str());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextWrapped("NextXI is a modernization project built to replace legacy hook architectures with multi-threaded, hardware-accelerated Direct3D hook proxies.");
        ImGui::Spacing();
        ImGui::Text("GitHub Repository      : https://github.com/voliathon/NextXI");

        ImGui::Spacing();

        // The slick, unobtrusive copyright drop-down
        if (ImGui::CollapsingHeader("Third-Party Acknowledgments"))
        {
            ImGui::Text("NextXI is made possible by open-source contributions:");
            ImGui::BulletText("Windower Dev Team (Resources & Legacy Lua Libraries)");
            ImGui::BulletText("dgVoodoo2 (DirectX Wrapper)");
            ImGui::BulletText("LuaJIT (Just-In-Time Compiler)");
            ImGui::BulletText("ImGui (Graphical Interface)");
        }

        ImGui::EndTabItem();
    }
}
