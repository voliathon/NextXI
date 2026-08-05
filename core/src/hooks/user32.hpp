#ifndef WINDOWER_HOOKS_USER32_HPP
#define WINDOWER_HOOKS_USER32_HPP

#include <windows.h>

namespace windower::user32
{

void install();
void uninstall() noexcept;

}

#endif