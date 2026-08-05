#ifndef WINDOWER_ADDON_MODULES_COMMAND_HPP
#define WINDOWER_ADDON_MODULES_COMMAND_HPP

#include "addon/lua.hpp"
#include "command_manager.hpp"

namespace windower
{

bool trigger_unknown_command(std::u8string_view command, command_source source);

int load_command_module(lua::state);

}

#endif