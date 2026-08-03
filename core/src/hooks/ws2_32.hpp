#ifndef WINDOWER_HOOKS_WS2_32_HPP
#define WINDOWER_HOOKS_WS2_32_HPP

#include <winsock2.h>

namespace windower::ws2_32
{

void install();
void uninstall() noexcept;

}

#endif
