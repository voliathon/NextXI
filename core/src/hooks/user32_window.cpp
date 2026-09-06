#include "user32_internal.hpp"
#include "user32_window_split.hpp"
#include "user32_window_find.hpp"

void windower::user32::install_window()
{
    install_window_class();
    install_window_msg();
    install_window_create();
    install_window_find();
}

void windower::user32::uninstall_window() noexcept
{
    uninstall_window_class();
    uninstall_window_msg();
    uninstall_window_create();
    uninstall_window_find();
}
