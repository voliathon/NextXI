#pragma once
#include <windows.h>
#include <string_view>
#include <span>

namespace windower::memory
{
    /**
     * Hooks a specific imported function in a module's IAT.
     * @param target_module The module whose IAT to modify (e.g., GetModuleHandle(nullptr)).
     * @param dll_name The imported DLL name (e.g., "user32.dll").
     * @param func_name The imported function name (e.g., "EnumWindows").
     * @param proxy_func The replacement callback address.
     * @param original_func Optional pointer to store the original address.
     * @return true if successfully hooked, false otherwise.
     */
    bool hook_iat(
        ::HMODULE target_module,
        std::string_view dll_name,
        std::string_view func_name,
        void* proxy_func,
        void** original_func = nullptr
    ) noexcept;
}
