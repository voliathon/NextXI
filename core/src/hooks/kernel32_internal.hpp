#pragma once
#include <windows.h>

namespace windower::kernel32
{
    void install_sync();
    void uninstall_sync() noexcept;

    void install_process();
    void uninstall_process() noexcept;

    void install_misc();
    void uninstall_misc() noexcept;
}
