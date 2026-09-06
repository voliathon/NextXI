#include "ui/system_diagnostics.hpp"
#include "core.hpp" // Added to access core::instance().settings
#include <imgui.h>
#include <windows.h>
#include <string>
#include <vector>
#include <array>
#include <cctype>

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
        // FFXI is a DX8 game. dgVoodoo2 wraps it by pretending to be d3d8.dll.
        HMODULE hMod = ::GetModuleHandleA("d3d8.dll");
        if (!hMod) return "";

        // Safe array to prevent C26485 decay warnings
        std::array<char, MAX_PATH> path_buf{};
        if (::GetModuleFileNameA(hMod, path_buf.data(), MAX_PATH) == 0) return "";

        // Convert path to lowercase to safely check directories
        std::string lower_path = path_buf.data();
        for (auto& c : lower_path) c = gsl::narrow_cast<char>(tolower(c));

        // If the DLL loaded from the Windows system folder, the Launcher did NOT wrap it.
        if (lower_path.find("system32") != std::string::npos || lower_path.find("syswow64") != std::string::npos) {
            return "";
        }

        // If we made it here, the Launcher successfully injected a local d3d8.dll wrapper!
        DWORD dummy = 0;
        DWORD const size = ::GetFileVersionInfoSizeA(path_buf.data(), &dummy);
        if (size == 0) return " (Custom Build)";

        std::vector<BYTE> buffer(size);
        if (!::GetFileVersionInfoA(path_buf.data(), 0, size, buffer.data())) return " (Custom Build)";

        VS_FIXEDFILEINFO* file_info = nullptr;
        UINT file_info_len = 0;
        if (::VerQueryValueA(buffer.data(), "\\", reinterpret_cast<LPVOID*>(&file_info), &file_info_len))
        {
            auto const ms = file_info->dwFileVersionMS;
            auto const ls = file_info->dwFileVersionLS;
            return " v" + std::to_string((ms >> 16) & 0xFFFF) + "." +
                std::to_string(ms & 0xFFFF) + "." + std::to_string((ls >> 16) & 0xFFFF);
        }
        return " (Custom Build)";
    }
}

void windower::ui::system_diagnostics::render_about_tab()
{
    if (ImGui::BeginTabItem("About"))
    {
        ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "NextXI Engine Architecture");
        ImGui::Separator();
        ImGui::Spacing();

        auto const& settings = core::instance().settings;
        std::string graphics_engine = "Unknown";
        std::string voodoo_ver = get_dgvoodoo_version();

        // Map the new launcher settings to the UI display strings
        if (settings.graphics_engine == u8"Vanilla")
        {
            graphics_engine = "DirectX 8 (Vanilla)";
        }
        else if (settings.graphics_engine == u8"Direct3D11")
        {
            graphics_engine = "NextXI (DirectX 11 via dgVoodoo2)" + voodoo_ver;
        }
        else if (settings.graphics_engine == u8"Direct3D12")
        {
            graphics_engine = "NextXI (DirectX 12 via dgVoodoo2)" + voodoo_ver;
        }
        else if (!voodoo_ver.empty())
        {
            // Fallback just in case
            graphics_engine = "NextXI (dgVoodoo2)" + voodoo_ver;
        }

        std::string lua_version_str = "LuaJIT (Sandbox)";
#if defined(LUAJIT_VERSION)
        lua_version_str = std::string(LUAJIT_VERSION) + " (Sandbox)";
#elif defined(LUA_RELEASE)
        lua_version_str = std::string(LUA_RELEASE) + " (Sandbox)";
#endif

        ImGui::Text("Core Engine Build Date : %s %s", __DATE__, __TIME__);

        // Formatted engine output
        ImGui::Text("Graphics Renderer      : ");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "%s", graphics_engine.c_str());

        // Dynamic VRAM output (Only shows for NextXI wrappers)
        if (settings.graphics_engine != u8"Vanilla")
        {
            ImGui::Text("Simulated VRAM         : ");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "%d MB", settings.vram_allocation);
        }

        // Frame Rate Cap Readout
        ImGui::Text("Target Frame Rate      : ");
        ImGui::SameLine();
        if (settings.fps_divisor == 1) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "60 FPS (Unlocked)");
        }
        else if (settings.fps_divisor == 0) {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Uncapped (Experimental)");
        }
        else {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "30 FPS (Default)");
        }

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
