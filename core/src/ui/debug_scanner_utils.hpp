#pragma once
#pragma once
#include <cstdint>
#include <windows.h>

namespace windower::ui::debug_scanner::utils
{
    inline bool safe_read_int32(uintptr_t addr, int32_t& out_val) noexcept {
        __try {
            out_val = *reinterpret_cast<int32_t*>(addr);
            return true;
        }
        __except (1) { return false; }
    }

    inline bool safe_read_uint8(uint8_t* addr, uint8_t& out_val) noexcept {
        __try {
            out_val = *addr;
            return true;
        }
        __except (1) { return false; }
    }

    inline bool write_memory_int32(uintptr_t addr, int32_t val) noexcept {
        DWORD old_protect;
        if (::VirtualProtect(reinterpret_cast<void*>(addr), sizeof(int32_t), PAGE_READWRITE, &old_protect)) {
            *reinterpret_cast<int32_t*>(addr) = val;
            ::VirtualProtect(reinterpret_cast<void*>(addr), sizeof(int32_t), old_protect, &old_protect);
            return true;
        }
        return false;
    }
}
