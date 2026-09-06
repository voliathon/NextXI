#include "hooks/user32.hpp"
#include "user32_internal.hpp"

void windower::user32::install()
{
    install_input();
    install_window();
}

void windower::user32::uninstall() noexcept
{
    uninstall_input();
    uninstall_window();
}
