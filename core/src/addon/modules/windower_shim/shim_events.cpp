#include "shim.hpp"
#include "command_manager.hpp"
#include <lua.hpp>
#include <gsl/gsl>
#include <string>
#include <vector>
#include <memory>

namespace windower::w4_shim {

    // Persistent tag to prevent command_manager from auto-purging shim commands
    static auto g_shim_tag = std::make_shared<int>(1);

    static int lua_register_event(lua_State* L) {
        const char* event_name = luaL_checkstring(L, 1);
        if (!lua_isfunction(L, 2)) {
            return luaL_error(L, "register_event: expected function as second argument");
        }

        std::string const reg_key = std::string("w4_evt_") + event_name;
        lua_pushvalue(L, 2);
        lua_setfield(L, LUA_REGISTRYINDEX, reg_key.c_str());

        if (std::string(event_name) == "addon command") {
            lua_getglobal(L, "_addon");
            if (lua_istable(L, -1)) {
                lua_getfield(L, -1, "commands");
                if (lua_istable(L, -1)) {
                    lua_pushnil(L);
                    while (lua_next(L, -2) != 0) {
                        if (lua_isstring(L, -1)) {
                            std::string cmd = lua_tostring(L, -1);
                            std::u8string u8cmd(reinterpret_cast<const char8_t*>(cmd.c_str()));

                            command_manager::instance().register_command(
                                command_manager::layer::addon,
                                u8"w4_shim",
                                u8cmd,
                                [L, reg_key](std::vector<std::u8string> const& args, command_source) {
                                    lua_getfield(L, LUA_REGISTRYINDEX, reg_key.c_str());
                                    if (lua_isfunction(L, -1)) {
                                        for (auto const& arg : args) {
                                            lua_pushstring(L, reinterpret_cast<const char*>(arg.c_str()));
                                        }
                                        lua_pcall(L, gsl::narrow_cast<int>(args.size()), 0, 0);
                                    }
                                    else {
                                        lua_pop(L, 1);
                                    }
                                },
                                false,
                                g_shim_tag // Active shared_ptr tag prevents unregistering
                            );
                        }
                        lua_pop(L, 1);
                    }
                }
                lua_pop(L, 1); // pop commands table
            }
            lua_pop(L, 1); // pop _addon table
        }
        return 0;
    }

    void register_event_bindings(lua_State* L) {
        lua_pushcfunction(L, lua_register_event);
        lua_setfield(L, -2, "register_event");
    }
}
