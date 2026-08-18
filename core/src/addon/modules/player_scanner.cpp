#include "player_scanner.hpp"
#include <windows.h>
#include <cstring>
#include <cstdio>
#include <string>
#include <chrono>

namespace windower::player_scanner
{
    static char g_cached_player_name[24] = { 0 };
    static char g_cached_server_name[32] = { 0 };
    static bool g_player_found = false;
    static char g_diag_msg[256] = "Scanning memory in background...";

#pragma warning(push)
#pragma warning(disable: 6320 26429 26446 26462 26471 26472 26481 26482 26485 26493 26496)

    static const char* k_servers[] = {
        "Bahamut", "Asura", "Odin", "Shiva", "Fenrir", "Sylph", "Valefor", "Leviathan",
        "Carbuncle", "Diabolos", "Caitsith", "Quetzalcoatl", "Siren", "Ragnarok",
        "Cerberus", "Bismarck", "Lakshmi", "Phoenix", "Eden", "HorizonXI", "CatsEyeXI"
    };

    static void** find_entity_array() noexcept {
        HMODULE hMod = ::GetModuleHandleW(L"FFXiMain.dll");
        if (!hMod) return nullptr;

        uint8_t* base = reinterpret_cast<uint8_t*>(hMod);
        PIMAGE_DOS_HEADER dos = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
        PIMAGE_NT_HEADERS nt = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos->e_lfanew);
        DWORD size = nt->OptionalHeader.SizeOfImage;

        MEMORY_BASIC_INFORMATION mbi;
        for (uint8_t* curr = base; curr < base + size; curr += mbi.RegionSize) {
            if (!::VirtualQuery(curr, &mbi, sizeof(mbi))) break;
            if (mbi.State != MEM_COMMIT || (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))) continue;

            uint8_t* region_end = reinterpret_cast<uint8_t*>(mbi.BaseAddress) + mbi.RegionSize;
            uint8_t* p = reinterpret_cast<uint8_t*>(mbi.BaseAddress);

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

    static bool safe_read_entity_name(void** arr, int idx, char* out_name) noexcept {
        __try {
            void* ent = arr[idx];
            if (!ent) return false;
            std::memcpy(out_name, reinterpret_cast<char*>(ent) + 0x7C, 24);
            return true;
        }
        __except (1) { return false; }
    }

    static bool execute_deep_scan() noexcept {
        HMODULE hMod = ::GetModuleHandleW(L"FFXiMain.dll");
        if (!hMod) {
            strcpy_s(g_diag_msg, "FAILED: Could not find FFXiMain.dll");
            return false;
        }

        void** ent_arr = find_entity_array();
        if (!ent_arr) {
            strcpy_s(g_diag_msg, "Scanning... Waiting for 3D world to initialize.");
            return false;
        }

        uint8_t* base = reinterpret_cast<uint8_t*>(hMod);
        PIMAGE_DOS_HEADER dos = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
        PIMAGE_NT_HEADERS nt = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos->e_lfanew);
        DWORD size = nt->OptionalHeader.SizeOfImage;

        MEMORY_BASIC_INFORMATION mbi;
        for (uint8_t* curr = base; curr < base + size; curr += mbi.RegionSize) {
            if (!::VirtualQuery(curr, &mbi, sizeof(mbi))) break;
            if (mbi.State != MEM_COMMIT || (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))) continue;

            uint8_t* region_end = reinterpret_cast<uint8_t*>(mbi.BaseAddress) + mbi.RegionSize;

            for (uint8_t* p = reinterpret_cast<uint8_t*>(mbi.BaseAddress) + 16; p < region_end - 16; ++p) {
                char c = static_cast<char>(*p);
                if (c != 'B' && c != 'A' && c != 'O' && c != 'S' && c != 'F' && c != 'V' &&
                    c != 'L' && c != 'C' && c != 'D' && c != 'Q' && c != 'R' && c != 'P' &&
                    c != 'E' && c != 'H') continue;

                for (const char* server : k_servers) {
                    if (c == server[0]) {
                        size_t s_len = strlen(server);
                        if (std::memcmp(p, server, s_len) == 0 && p[s_len] == '\0') {
                            uint8_t* name_ptr = p - 16;
                            if (name_ptr[0] >= 'A' && name_ptr[0] <= 'Z') {
                                bool valid = false;
                                for (int i = 1; i < 16; ++i) {
                                    if (name_ptr[i] == '\0') { valid = true; break; }
                                    if (name_ptr[i] < 32 || name_ptr[i] > 126) break;
                                }

                                if (valid) {
                                    char potential_name[24] = { 0 };
                                    std::memcpy(potential_name, name_ptr, 16);
                                    potential_name[15] = '\0';

                                    bool spawned_in_world = false;
                                    for (int i = 1024; i < 2304; ++i) {
                                        char ent_name[24] = { 0 };
                                        if (safe_read_entity_name(ent_arr, i, ent_name)) {
                                            if (std::strcmp(potential_name, ent_name) == 0) {
                                                spawned_in_world = true;
                                                break;
                                            }
                                        }
                                    }

                                    if (spawned_in_world) {
                                        strcpy_s(g_cached_player_name, 24, potential_name);
                                        strcpy_s(g_cached_server_name, 32, server);
                                        return true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        strcpy_s(g_diag_msg, "Scanning... Awaiting character to spawn in-game.");
        return false;
    }

    void trigger_manual_scan() noexcept {
        if (execute_deep_scan()) {
            g_player_found = true;
            sprintf_s(g_diag_msg, "SUCCESS: Found Character '%s' on Server '%s'", g_cached_player_name, g_cached_server_name);
        }
    }

    void reset_scan() noexcept {
        g_player_found = false;
        g_cached_player_name[0] = '\0';
        g_cached_server_name[0] = '\0';
        strcpy_s(g_diag_msg, "Scanning memory in background...");
    }

    char const* get_diagnostic_message() noexcept {
        return g_diag_msg;
    }

    char const* get_local_player_json() noexcept {

        // --- THE SESSION VERIFICATION LOOP ---
        if (g_player_found) {
            static auto last_verify = std::chrono::steady_clock::now();
            if (std::chrono::steady_clock::now() - last_verify > std::chrono::seconds(2)) {

                void** ent_arr = find_entity_array();
                bool found_in_pc = false;
                bool found_in_npc = false;

                if (ent_arr) {
                    for (int i = 1024; i < 2304; ++i) {
                        char ent_name[24] = { 0 };
                        if (safe_read_entity_name(ent_arr, i, ent_name)) {
                            if (std::strcmp(g_cached_player_name, ent_name) == 0) { found_in_pc = true; break; }
                        }
                    }

                    if (!found_in_pc) {
                        for (int i = 0; i < 1024; ++i) {
                            char ent_name[24] = { 0 };
                            if (safe_read_entity_name(ent_arr, i, ent_name)) {
                                if (std::strcmp(g_cached_player_name, ent_name) == 0) { found_in_npc = true; break; }
                            }
                        }
                    }
                }

                static int missing_counter = 0;

                if (found_in_pc) {
                    // Player is safely in-game.
                    missing_counter = 0;
                }
                else if (found_in_npc) {
                    // Player is explicitly in the NPC block -> WE LOGGED OUT TO CHARACTER SELECT!
                    reset_scan();
                }
                else {
                    // Player is completely missing -> WE ARE ZONING OR ON THE OVERVIEW MENU
                    missing_counter++;
                    if (missing_counter >= 5) { // 10 seconds of missing = drop the session
                        reset_scan();
                        missing_counter = 0;
                    }
                }

                last_verify = std::chrono::steady_clock::now();
            }
        }

        // --- THE ACQUISITION SCANNER ---
        if (!g_player_found) {
            static auto last_auth_scan = std::chrono::steady_clock::now() - std::chrono::seconds(5);

            if (std::chrono::steady_clock::now() - last_auth_scan > std::chrono::seconds(2)) {
                if (execute_deep_scan()) {
                    g_player_found = true;
                    sprintf_s(g_diag_msg, "SUCCESS: Found Character '%s' on Server '%s'", g_cached_player_name, g_cached_server_name);
                }
                last_auth_scan = std::chrono::steady_clock::now();
            }
        }

        if (g_player_found) {
            static std::string player_json;
            player_json = "{ \"name\": \"";
            player_json += g_cached_player_name;
            player_json += "\", \"hp\": 1000, \"mp\": 500, \"tp\": 3000, "
                "\"main_job_id\": 1, \"main_job_level\": 99, \"sub_job_id\": 4, \"sub_job_level\": 49 }";
            return player_json.c_str();
        }
        return nullptr;
    }
#pragma warning(pop)
}
