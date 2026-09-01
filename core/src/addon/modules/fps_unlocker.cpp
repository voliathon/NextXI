#include "fps_unlocker.hpp"
#include <windows.h>
#include <cstdio>
#include <vector>
#include <string>

namespace windower::addon::modules::fps
{
    static int32_t* s_framerate_divisor_ptr = nullptr;

    // Fully global IDA pattern scanner using VirtualQuery (No blind spots)
    static uintptr_t scan_signature(const char* pattern) noexcept {
        HMODULE hMod = ::GetModuleHandleW(L"FFXiMain.dll");
        if (!hMod) return 0;

        // Parse IDA pattern
        std::vector<int> pattern_bytes;
        const char* current = pattern;
        while (*current) {
            if (*current == '?') {
                pattern_bytes.push_back(-1);
                current++;
                if (*current == '?') current++;
            }
            else if (isxdigit(static_cast<unsigned char>(*current))) {
                pattern_bytes.push_back(strtol(current, nullptr, 16));
                while (isxdigit(static_cast<unsigned char>(*current))) current++;
            }
            else {
                current++;
            }
        }

        uint8_t* base = (uint8_t*)hMod;
        PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
        PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(base + dos->e_lfanew);
        DWORD size = nt->OptionalHeader.SizeOfImage; // Scan the ENTIRE mapped image, not just Code block

        size_t pattern_len = pattern_bytes.size();
        MEMORY_BASIC_INFORMATION mbi;

        // Sweep every memory region belonging to FFXiMain.dll
        for (uint8_t* curr = base; curr < base + size; curr += mbi.RegionSize) {
            if (!::VirtualQuery(curr, &mbi, sizeof(mbi))) break;
            if (mbi.State != MEM_COMMIT) continue;
            if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) continue;

            uint8_t* region_end = (uint8_t*)mbi.BaseAddress + mbi.RegionSize;
            uint8_t* scan_start = (uint8_t*)mbi.BaseAddress;

            // Prevent out-of-bounds scanning at the very edge of a region
            if (region_end - scan_start < pattern_len) continue;

            for (uint8_t* p = scan_start; p <= region_end - pattern_len; ++p) {
                bool found = true;
                for (size_t j = 0; j < pattern_len; ++j) {
                    if (pattern_bytes[j] != -1 && p[j] != static_cast<uint8_t>(pattern_bytes[j])) {
                        found = false;
                        break;
                    }
                }
                if (found) {
                    return reinterpret_cast<uintptr_t>(p);
                }
            }
        }
        return 0;
    }

    bool initialize_fps_pointer(logger_callback const& log) noexcept {
        if (s_framerate_divisor_ptr) return true;

        HMODULE hMod = ::GetModuleHandleW(L"FFXiMain.dll");
        if (!hMod) {
            log(u8"FPS UNLOCKER: FFXiMain.dll not found.");
            return false;
        }

        // Use strong static offset from resolver
        uintptr_t static_offset = 0x9C47C4;

        // Read dynamic Config base address from static pointer
        uintptr_t config_base = *reinterpret_cast<uintptr_t*>((uint8_t*)hMod + static_offset);

        if (!config_base) {
            log(u8"FPS UNLOCKER: Config base null. Engine not ready.");
            return false;
        }

        // Add 0x30 offset to reach Framerate Divisor
        s_framerate_divisor_ptr = reinterpret_cast<int32_t*>(config_base + 0x30);

        char buf[256];
        sprintf_s(buf, "FPS UNLOCKER: Locked! Divisor at 0x%p", (void*)s_framerate_divisor_ptr);
        log(reinterpret_cast<const char8_t*>(buf));
        return true;
    }

    void set_fps_divisor(int32_t divisor, logger_callback const& log) noexcept {
        if (!s_framerate_divisor_ptr) {
            if (!initialize_fps_pointer(log)) return;
        }

        DWORD old_protect;
        if (::VirtualProtect(s_framerate_divisor_ptr, sizeof(int32_t), PAGE_READWRITE, &old_protect)) {
            *s_framerate_divisor_ptr = divisor;
            ::VirtualProtect(s_framerate_divisor_ptr, sizeof(int32_t), old_protect, &old_protect);

            char buf[128];
            int target_fps = (divisor == 1) ? 60 : (divisor == 2) ? 30 : 0;
            if (divisor == 0) sprintf_s(buf, "FPS UNLOCKER: Divisor set to %d (Uncapped).", divisor);
            else sprintf_s(buf, "FPS UNLOCKER: Divisor set to %d (%d FPS).", divisor, target_fps);

            log(reinterpret_cast<const char8_t*>(buf));
        }
        else {
            log(u8"FPS UNLOCKER: Failed to change memory protection for pointer.");
        }
    }
}
