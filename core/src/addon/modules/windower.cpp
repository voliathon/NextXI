#include "windower.hpp"
#include "addon/addon.hpp"
#include "addon/package_manager.hpp"
#include "addon/lua.hpp"
#include "addon/modules/windower.lua.hpp"
#include "addon/profile_manager.hpp"
#include "addon/unsafe.hpp" 
#include <lua.hpp>
#include "core.hpp"
#include "utility.hpp"
#include "utilities/paths.hpp"
#include "version.hpp"
#include "player_scanner.hpp" 
#include "../../scanner.hpp"
#include <windows.h>
#include <cstring>
#include <string>
#include <span>
#include <filesystem>
#include <fstream>
#include <chrono>

namespace {
    extern "C" char const* get_package_list_ffi()
    {
        static std::string result;
        result.clear();
        auto const& pm = windower::core::instance().package_manager;
        for (auto const& pkg : pm->installed_packages())
        {
            result.append(reinterpret_cast<char const*>(pkg->name().c_str()));
            result.append("|");
            result.append(reinterpret_cast<char const*>(windower::to_u8string(pkg->version()).c_str()));
            result.append("|");
            result.append(reinterpret_cast<char const*>(pkg->path().u8string().c_str()));
            result.append("|");
            const bool has_readme = std::filesystem::exists(pkg->path() / u8"README.md");
            result.append(has_readme ? "1" : "0");
            result.append(";");
        }
        return result.c_str();
    }

    extern "C" char const* read_market_file_ffi(char const* filename)
    {
        static std::string content;
        content.clear();
        auto dir = windower::user_path() / u8"addons" / u8"NextXIMarket";
        auto path = dir / reinterpret_cast<const char8_t*>(filename);

        std::ifstream file(path, std::ios::binary);
        if (file) {
            content.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        }
        return content.c_str();
    }

    extern "C" void write_market_file_ffi(char const* filename, char const* data)
    {
        auto dir = windower::user_path() / u8"addons" / u8"NextXIMarket";
        std::filesystem::create_directories(dir);
        auto path = dir / reinterpret_cast<const char8_t*>(filename);
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (file) file << data;
    }

    extern "C" char const* get_package_readme_ffi(char const* pkg_name)
    {
        static std::string content;
        content.clear();
        std::u8string u8_name = reinterpret_cast<const char8_t*>(pkg_name);
        auto const& pm = windower::core::instance().package_manager;
        auto pkg = pm->get_package(u8_name);

        if (pkg) {
            auto readme_path = pkg->path() / u8"README.md";
            std::ifstream file(readme_path, std::ios::binary);
            if (file) {
                content.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            }
        }
        return content.c_str();
    }

    extern "C" char const* get_ffxi_player_ffi()
    {
        return windower::player_scanner::get_local_player_json();
    }

    extern "C" char const* get_ffxi_items_ffi()
    {
        static std::string items_json =
            "{ \"inventory\": [ { \"id\": 4100, \"count\": 1 }, { \"id\": 4101, \"count\": 99 } ], "
            "\"equipment\": { \"main\": 4100, \"sub\": 0 } }";
        return items_json.c_str();
    }

    extern "C" char const* get_ffxi_spells_ffi()
    {
        static std::string spells_json = "[ 1, 2, 3, 4, 5 ]";
        return spells_json.c_str();
    }

    extern "C" char const* get_ffxi_entities_ffi()
    {
        return "[]";
    }

#pragma warning(push)
#pragma warning(disable: 26446)
    extern "C" void project_ffi(float const* in_pos, float* out_screen)
    {
        auto const& core = windower::core::instance();
        auto const& v = core.view_matrix;
        auto const& p = core.projection_matrix;
        auto const& vp = core.viewport;

        std::span<float const, 3> const in_span{ in_pos, 3 };
        std::span<float, 2> const out_span{ out_screen, 2 };

        float const vx = in_span[0] * v.m[0][0] + in_span[1] * v.m[1][0] + in_span[2] * v.m[2][0] + v.m[3][0];
        float const vy = in_span[0] * v.m[0][1] + in_span[1] * v.m[1][1] + in_span[2] * v.m[2][1] + v.m[3][1];
        float const vz = in_span[0] * v.m[0][2] + in_span[1] * v.m[1][2] + in_span[2] * v.m[2][2] + v.m[3][2];
        float const vw = in_span[0] * v.m[0][3] + in_span[1] * v.m[1][3] + in_span[2] * v.m[2][3] + v.m[3][3];
        float const cx = vx * p.m[0][0] + vy * p.m[1][0] + vz * p.m[2][0] + vw * p.m[3][0];
        float const cy = vx * p.m[0][1] + vy * p.m[1][1] + vz * p.m[2][1] + vw * p.m[3][1];
        float const cw = vx * p.m[0][3] + vy * p.m[1][3] + vz * p.m[2][3] + vw * p.m[3][3];

        if (cw < 0.1f)
        {
            out_span[0] = -1.0f;
            out_span[1] = -1.0f;
            return;
        }

        float const ndcx = cx / cw;
        float const ndcy = cy / cw;

        out_span[0] = vp.X + (1.0f + ndcx) * vp.Width / 2.0f;
        out_span[1] = vp.Y + (1.0f - ndcy) * vp.Height / 2.0f;
    }
#pragma warning(pop)
}

int windower::load_windower_module(lua::state s) {
    lua::stack_guard guard{ s };

    lua::load(guard, lua_windower_source, u8"core.windower");

    lua::push(guard, WINDOWER_VERSION_STRING);
    lua::push(guard, WINDOWER_VERSION_MAJOR);
    lua::push(guard, WINDOWER_VERSION_MINOR);
    lua::push(guard, WINDOWER_VERSION_BUILD_STRING);
    lua::push(guard, WINDOWER_BUILD_TAG_STRING);

    auto const scripts = windower_path() / u8"scripts";

    lua::push(guard, client_path().u8string());
    lua::push(guard, scripts.u8string());

    auto const& core = core::instance();

    lua::push(guard, core.settings.window_bounds.size.width);
    lua::push(guard, core.settings.window_bounds.size.height);
    lua::push(guard, core.settings.ui_size.width);
    lua::push(guard, core.settings.ui_size.height);

    lua::push(guard, core.client_hwnd);

    if (auto const package = addon::get_package(s))
    {
        auto const settings = settings_path() / u8"settings" / package->name();
        auto const user = user_path() / u8"addons" / package->name();

        lua::push(guard, settings.u8string());
        lua::push(guard, user.u8string());
        lua::push(guard, package->path().u8string());
        lua::push(guard, package->name());
    }
    else
    {
        lua::push(guard, lua::nil);
        lua::push(guard, lua::nil);
        lua::push(guard, lua::nil);
        lua::push(guard, lua::nil);
    }

    lua::push(guard, &get_package_list_ffi);
    lua::push(guard, &get_package_readme_ffi);
    lua::push(guard, &read_market_file_ffi);
    lua::push(guard, &write_market_file_ffi);

    lua::push(guard, &get_ffxi_player_ffi);
    lua::push(guard, &get_ffxi_items_ffi);
    lua::push(guard, &get_ffxi_spells_ffi);
    lua::push(guard, &get_ffxi_entities_ffi);
    lua::push(guard, &project_ffi);

    lua::call(guard, 25);

    // ==========================================
     // INJECT PROFILE MANAGER TO GLOBAL core.profile 
     // ==========================================
    auto L = lua::unsafe::unwrap(s);

    lua_getglobal(L, "core");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "core");
    }

    lua_pushstring(L, "profile");
    lua_newtable(L);

    // Use NextXI's safe wrapper to push the functions so the types match perfectly
    lua::push(guard, u8"set_autoload");
    lua::push(guard, windower::profile_manager::lua_set_autoload);
    lua::raw_set(guard, -3);

    lua::push(guard, u8"get_autoload");
    lua::push(guard, windower::profile_manager::lua_get_autoload);
    lua::raw_set(guard, -3);

    lua_rawset(L, -3);  // core["profile"] = table
    lua_pop(L, 1);      // pop "core"

    return guard.release();
}
