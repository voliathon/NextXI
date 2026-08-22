#include "addon/profile_manager.hpp"
#include "core.hpp"
#include "addon/unsafe.hpp"
#include "addon/lua.hpp"
#include <lua.hpp>
#include <nlohmann/json.hpp>
#include <fstream>

namespace windower
{
    profile_manager& profile_manager::instance() {
        static profile_manager s_instance;
        return s_instance;
    }

    void profile_manager::set_character(std::u8string_view character_name) {
        std::lock_guard<std::mutex> lock{ m_mutex };
        m_character_name = character_name;
    }

    std::u8string profile_manager::get_character() const {
        std::lock_guard<std::mutex> lock{ m_mutex };
        return m_character_name;
    }

    std::filesystem::path profile_manager::get_profile_path() const {
        if (m_character_name.empty()) return {};
        // Dynamically routes to NextXI/profiles/PlayerName/autoload.json
        return core::instance().settings.user_path / "profiles" / std::filesystem::path(m_character_name) / "autoload.json";
    }

    boot_config profile_manager::get_addon_config(std::u8string_view addon_name) {
        std::lock_guard<std::mutex> lock{ m_mutex };

        auto path = get_profile_path();
        if (path.empty() || !std::filesystem::exists(path)) return {};

        try {
            std::ifstream file(path);
            nlohmann::json j;
            file >> j;

            std::string addon_str(addon_name.begin(), addon_name.end());
            if (j.contains(addon_str)) {
                return {
                    j[addon_str].value("auto_load", false),
                    j[addon_str].value("launch_hidden", false)
                };
            }
        }
        catch (...) {
            // Failsafe: If JSON is invalid/corrupt, fail silently to default values
        }
        return {};
    }

    void profile_manager::set_addon_config(std::u8string_view addon_name, bool auto_load, bool launch_hidden) {
        std::lock_guard<std::mutex> lock{ m_mutex };

        auto path = get_profile_path();
        if (path.empty()) return;

        std::filesystem::create_directories(path.parent_path());

        nlohmann::json j;
        if (std::filesystem::exists(path)) {
            try {
                std::ifstream file(path);
                file >> j;
            }
            catch (...) {
                j = nlohmann::json::object();
            }
        }
        else {
            j = nlohmann::json::object();
        }

        std::string addon_str(addon_name.begin(), addon_name.end());
        j[addon_str] = {
            {"auto_load", auto_load},
            {"launch_hidden", launch_hidden}
        };

        try {
            std::ofstream file(path);
            file << j.dump(4);
        }
        catch (...) {}
    }

    // ========================================================================
    // LUA API BRIDGE
    // ========================================================================

    int profile_manager::lua_set_autoload(lua::state s) {
        auto L = lua::unsafe::unwrap(s);
        const char* name = ::lua_tolstring(L, 1, nullptr);
        bool auto_load = ::lua_toboolean(L, 2) != 0;
        bool launch_hidden = ::lua_toboolean(L, 3) != 0;

        if (name) {
            instance().set_addon_config(reinterpret_cast<const char8_t*>(name), auto_load, launch_hidden);
        }
        return 0;
    }

    int profile_manager::lua_get_autoload(lua::state s) {
        auto L = lua::unsafe::unwrap(s);
        const char* name = ::lua_tolstring(L, 1, nullptr);

        if (name) {
            auto config = instance().get_addon_config(reinterpret_cast<const char8_t*>(name));
            ::lua_pushboolean(L, config.auto_load ? 1 : 0);
            ::lua_pushboolean(L, config.launch_hidden ? 1 : 0);
            return 2;
        }
        return 0;
    }

    void profile_manager::bind(lua::stack_guard& guard) {
        auto bind_fn = [&](const char8_t* name, auto func) {
            lua::push(guard, name);
            lua::push(guard, func);
            lua::raw_set(guard, -3);
            };

        bind_fn(u8"set_autoload", lua_set_autoload);
        bind_fn(u8"get_autoload", lua_get_autoload);
    }
}
