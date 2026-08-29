#pragma once
#include <windows.h>

namespace windower::user32
{
    void install_window_find();
    void uninstall_window_find() noexcept;
    void hook_window_find_module(::HMODULE mod) noexcept;
}
