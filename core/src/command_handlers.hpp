#ifndef WINDOWER_COMMAND_HANDLERS_HPP
#define WINDOWER_COMMAND_HANDLERS_HPP

#include "command_manager.hpp"

#include <string>
#include <vector>

namespace windower::command_handlers
{

void install(std::vector<std::u8string> const&, windower::command_source);
void uninstall(std::vector<std::u8string> const&, windower::command_source);
void update(std::vector<std::u8string> const&, windower::command_source);
void updateall(std::vector<std::u8string> const&, windower::command_source);
void load(std::vector<std::u8string> const&, windower::command_source);
void unload(std::vector<std::u8string> const&, windower::command_source);
void reload(std::vector<std::u8string> const&, windower::command_source);
void unloadall(std::vector<std::u8string> const&, windower::command_source);
void reloadall(std::vector<std::u8string> const&, windower::command_source);
void alias(std::vector<std::u8string> const&, windower::command_source);
void unalias(std::vector<std::u8string> const&, windower::command_source);
void bind(std::vector<std::u8string> const&, windower::command_source);
void unbind(std::vector<std::u8string> const&, windower::command_source);
void listbinds(std::vector<std::u8string> const&, windower::command_source);
void exec(std::vector<std::u8string> const&, windower::command_source);
void eval(std::vector<std::u8string> const&, windower::command_source);
void reset(std::vector<std::u8string> const&, windower::command_source);
void pkg(std::vector<std::u8string> const&, windower::command_source);
void nextwindow(std::vector<std::u8string> const&, windower::command_source);
void prevwindow(std::vector<std::u8string> const&, windower::command_source);

};

#endif