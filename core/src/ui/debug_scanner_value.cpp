#include "debug_scanner_value.hpp"
#include "debug_scanner_utils.hpp"
#include <windows.h>
#include <cstdio>
#include <vector>
#include <imgui.h>

namespace windower::ui::debug_scanner {
#pragma warning(push)
#pragma warning(disable: 6320 26429 26446 26462 26471 26472 26481 26482 26485 26493 26496)

    static std::vector<uintptr_t> s_scan_matches;
    static bool s_has_scanned = false;

    static void execute_first_scan(int32_t target_val, logger_callback const& log) noexcept {
        s_scan_matches.clear();
        s_has_scanned = true;

        HMODULE hMod = ::GetModuleHandleW(L"FFXiMain.dll");
        if (!hMod) { log(u8"ERROR: FFXiMain.dll not loaded."); return; }

        uint8_t* base = (uint8_t*)hMod;
        PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
        PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(base + dos->e_lfanew);
        DWORD size = nt->OptionalHeader.SizeOfImage;

        MEMORY_BASIC_INFORMATION mbi;
        for (uint8_t* curr = base; curr < base + size; curr += mbi.RegionSize) {
            if (!::VirtualQuery(curr, &mbi, sizeof(mbi))) break;
            if (mbi.State != MEM_COMMIT) continue;
            if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) continue;
            if (!(mbi.Protect & (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE))) continue;

            uint8_t* region_end = (uint8_t*)mbi.BaseAddress + mbi.RegionSize;
            for (uint8_t* p = (uint8_t*)mbi.BaseAddress; p <= region_end - 4; p += 4) {
                int32_t val = 0;
                if (utils::safe_read_int32(reinterpret_cast<uintptr_t>(p), val) && val == target_val) {
                    s_scan_matches.push_back(reinterpret_cast<uintptr_t>(p));
                }
            }
        }
        char buf[256];
        sprintf_s(buf, "FIRST SCAN: Found %zu matches for value '%d'.", s_scan_matches.size(), target_val);
        log(reinterpret_cast<const char8_t*>(buf));
    }

    static void execute_next_scan(int32_t target_val, logger_callback const& log) noexcept {
        if (!s_has_scanned || s_scan_matches.empty()) return;

        std::vector<uintptr_t> filtered;
        filtered.reserve(s_scan_matches.size());
        for (auto addr : s_scan_matches) {
            int32_t val = 0;
            if (utils::safe_read_int32(addr, val) && val == target_val) {
                filtered.push_back(addr);
            }
        }
        s_scan_matches = std::move(filtered);

        char buf[256];
        sprintf_s(buf, "NEXT SCAN: Filtered down to %zu matches for value '%d'.", s_scan_matches.size(), target_val);
        log(reinterpret_cast<const char8_t*>(buf));
    }

    void render_value_scanner_window(bool& show, logger_callback const& log, bool& focus_console) noexcept {
        if (!show) return;

        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Live Memory Value Scanner", &show)) {
            ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Search and filter 4-byte integers in FFXiMain.dll.");
            ImGui::Spacing();

            static int s_target_val = 2;
            ImGui::SetNextItemWidth(150);
            ImGui::InputInt("Value", &s_target_val);
            ImGui::SameLine();

            if (!s_has_scanned) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
                if (ImGui::Button("First Scan", ImVec2(100, 0))) { execute_first_scan(s_target_val, log); focus_console = true; }
                ImGui::PopStyleColor();
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 1.0f));
                if (ImGui::Button("Next Scan", ImVec2(100, 0))) { execute_next_scan(s_target_val, log); focus_console = true; }
                ImGui::PopStyleColor();
                ImGui::SameLine();
                if (ImGui::Button("Reset Scan", ImVec2(100, 0))) { s_scan_matches.clear(); s_has_scanned = false; log(u8"--- SCAN RESET ---"); }
            }

            ImGui::Spacing();
            ImGui::Text("Active Matches: %zu", s_scan_matches.size());

            if (!s_scan_matches.empty()) {
                ImGui::Spacing();
                if (ImGui::BeginChild("ScanResultsTable", ImVec2(0, 0), true)) {
                    size_t max_display = (s_scan_matches.size() < 100) ? s_scan_matches.size() : 100;
                    for (size_t i = 0; i < max_display; ++i) {
                        uintptr_t addr = s_scan_matches[i];
                        int32_t current_val = 0;
                        if (!utils::safe_read_int32(addr, current_val)) current_val = -999999;
                        ImGui::Text("0x%p | Current Value: %d", (void*)addr, current_val);
                    }
                }
                ImGui::EndChild();
            }
        }
        ImGui::End();
    }
#pragma warning(pop)
}
