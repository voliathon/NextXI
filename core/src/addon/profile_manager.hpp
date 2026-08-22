#ifndef WINDOWER_ADDON_PROFILE_MANAGER_HPP
#define WINDOWER_ADDON_PROFILE_MANAGER_HPP

#include <string>
#include <filesystem>
#include <mutex>
#include "addon/lua.hpp"

namespace windower
{
    // The data payload for our boot rules
    struct boot_config {
        bool auto_load = false;
        bool launch_hidden = false;
    };

    class profile_manager
    {
    public:
        static profile_manager& instance();

        // Called by the engine's memory scanner when a player logs in
        void set_character(std::u8string_view character_name);
        std::u8string get_character() const;

        // C++ Core Logic
        boot_config get_addon_config(std::u8string_view addon_name);
        void set_addon_config(std::u8string_view addon_name, bool auto_load, bool launch_hidden);

        // Lua API Bridge
        static int lua_set_autoload(lua::state s);
        static int lua_get_autoload(lua::state s);
        static void bind(lua::stack_guard& guard);

    private:
        profile_manager() = default;
        std::filesystem::path get_profile_path() const;

        mutable std::mutex m_mutex;
        std::u8string m_character_name;
    };
}

#endif
