#include "hooks/kernel32.hpp"
#include "kernel32_internal.hpp"

void windower::kernel32::install()
{
    install_sync();
    install_process();
    install_misc();
}

void windower::kernel32::uninstall() noexcept
{
    uninstall_sync();
    uninstall_process();
    uninstall_misc();
}
