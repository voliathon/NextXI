#include "debug_scanner_entity.hpp"
#include "debug_scanner_utils.hpp"
#include <windows.h>
#include <cstring>
#include <cstdio>
#include <imgui.h>

namespace windower::ui::debug_scanner
{
#pragma warning(push)
#pragma warning(disable: 6320 26429 26446 26462 26471 26472 26481 26482 26485 26493 26496)

    static void** find_entity_array() noexcept {
        HMODULE hMod = ::GetModuleHandleW(L"FFXiMain.dll");
        if (!hMod) return nullptr;
        uint8_t* base = (uint8_t*)hMod;
        PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
        PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(base + dos->e_lfanew);
        DWORD size = nt->OptionalHeader.SizeOfImage;
        MEMORY_BASIC_INFORMATION mbi;
        for (uint8_t* curr = base; curr < base + size; curr += mbi.RegionSize) {
            if (!::VirtualQuery(curr, &mbi, sizeof(mbi))) break;
            if (mbi.State != MEM_COMMIT) continue;
            if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) continue;
            uint8_t* region_end = (uint8_t*)mbi.BaseAddress + mbi.RegionSize;
            uint8_t* p = (uint8_t*)mbi.BaseAddress;
            for (; p < region_end - 9; ++p) {
                if (p[0] == 0x8B && p[1] == 0x56 && p[2] == 0x0C && p[3] == 0x8B &&
                    p[4] == 0x04 && p[5] == 0x2A && p[6] == 0x8B && p[7] == 0x04 && p[8] == 0x85) {
                    void** out = nullptr;
                    std::memcpy(&out, p + 9, sizeof(void**));
                    return out;
                }
            }
        }
        return nullptr;
    }

    static bool read_entity_slot(void** arr, int idx, void*& out_ent, char* out_data) noexcept {
        __try {
            out_ent = arr[idx];
            if (!out_ent) return false;
            std::memcpy(out_data, (char*)out_ent + 0x70, 32);
            return true;
        }
        __except (1) { return false; }
    }

    static void execute_entity_scan(logger_callback const& log) noexcept {
        log(u8"--- MANUAL MEMORY DUMP INITIATED ---");
        void** arr = find_entity_array();
        if (!arr) { log(u8"FATAL: Array signature not found in memory!"); return; }

        char buf[256];
        sprintf_s(buf, "SUCCESS: Array found at %p. Sweeping 2304 slots...", (void*)arr);
        log(reinterpret_cast<const char8_t*>(buf));

        int count = 0;
        for (int i = 0; i < 2304; ++i) {
            void* e = nullptr;
            char d[32] = { 0 };
            if (read_entity_slot(arr, i, e, d)) {
                char p[33] = { 0 };
                for (int j = 0; j < 32; ++j) p[j] = (d[j] >= 32 && d[j] <= 126) ? d[j] : '.';
                char line[256];
                sprintf_s(line, "Slot %04d | Ptr: %p | %s", i, e, p);
                log(reinterpret_cast<const char8_t*>(line));
                count++;
            }
        }
        sprintf_s(buf, "DUMP COMPLETE: %d active entities found.", count);
        log(reinterpret_cast<const char8_t*>(buf));
    }

    static void execute_status_scan(const char* search_name, logger_callback const& log) noexcept {
        char start_msg[256];
        sprintf_s(start_msg, "--- SEARCHING FOR STATUS BLOCK: '%s' ---", search_name);
        log(reinterpret_cast<const char8_t*>(start_msg));

        HMODULE hMod = ::GetModuleHandleW(L"FFXiMain.dll");
        if (!hMod) { log(u8"FATAL: Could not access FFXiMain.dll module."); return; }

        uint8_t* base = (uint8_t*)hMod;
        PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
        PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(base + dos->e_lfanew);
        DWORD size = nt->OptionalHeader.SizeOfImage;
        size_t name_len = strlen(search_name);
        int match_count = 0;
        MEMORY_BASIC_INFORMATION mbi;

        for (uint8_t* curr = base; curr < base + size; curr += mbi.RegionSize) {
            if (!::VirtualQuery(curr, &mbi, sizeof(mbi))) break;
            if (mbi.State != MEM_COMMIT) continue;
            if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) continue;

            uint8_t* region_end = (uint8_t*)mbi.BaseAddress + mbi.RegionSize;
            uint8_t* p = (uint8_t*)mbi.BaseAddress;
            for (; p < region_end - name_len; ++p) {
                if (std::memcmp(p, search_name, name_len) == 0) {
                    match_count++;
                    char buf[256];
                    sprintf_s(buf, "Match #%d at Ptr: %p", match_count, p);
                    log(reinterpret_cast<const char8_t*>(buf));

                    uint8_t* dump_start = p - 16;
                    for (int row = 0; row < 5; ++row) {
                        uint8_t* row_ptr = dump_start + (row * 16);
                        char hex_part[64] = { 0 };
                        char asc_part[32] = { 0 };
                        for (int b = 0; b < 16; ++b) {
                            uint8_t val = 0;
                            if (!utils::safe_read_uint8(row_ptr + b, val)) val = 0;
                            sprintf_s(hex_part + (b * 3), 4, "%02X ", val);
                            asc_part[b] = (val >= 32 && val <= 126) ? val : '.';
                        }
                        char line[256];
                        int offset = (row * 16) - 16;
                        char sign = (offset < 0) ? '-' : '+';
                        int display_offset = (offset < 0) ? -offset : offset;
                        sprintf_s(line, "  %c%02X | %s| %s", sign, display_offset, hex_part, asc_part);
                        log(reinterpret_cast<const char8_t*>(line));
                    }
                    log(u8"--------------------------------------------------");
                }
            }
        }
        char end_msg[256];
        sprintf_s(end_msg, "SEARCH COMPLETE: Found %d instances of '%s'.", match_count, search_name);
        log(reinterpret_cast<const char8_t*>(end_msg));
    }

    void render_entity_scanner_window(bool& show, logger_callback const& log, bool& focus_console) noexcept {
        if (!show) return;

        ImGui::SetNextWindowSize(ImVec2(500, 350), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Entity & Status Scanners", &show)) {
            ImGui::TextWrapped("1. Find Player Status Block");
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Scans memory for your exact name and dumps surrounding structures.");
            static char s_search_name[64] = "Please Select a Character";
            ImGui::InputText("Character Name", s_search_name, sizeof(s_search_name));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
            if (ImGui::Button("SCAN FOR PLAYER STATUS BLOCK", ImVec2(-1, 35))) {
                execute_status_scan(s_search_name, log);
                focus_console = true;
            }
            ImGui::PopStyleColor(2);

            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

            ImGui::TextWrapped("2. Find Global Entity Array");
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Extracts every active entity currently loaded in your zone.");
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            if (ImGui::Button("SCAN GLOBAL ENTITY ARRAY", ImVec2(-1, 35))) {
                execute_entity_scan(log);
                focus_console = true;
            }
            ImGui::PopStyleColor(2);
        }
        ImGui::End();
    }
#pragma warning(pop)
}
