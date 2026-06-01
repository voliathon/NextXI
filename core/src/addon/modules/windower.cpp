/*
 * Copyright © Windower Dev Team
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"),to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "windower.hpp"

#include "addon/addon.hpp"
#include "addon/package_manager.hpp"
#include "addon/lua.hpp"
#include "addon/modules/windower.lua.hpp"
#include "core.hpp"
#include "utility.hpp"
#include "version.hpp"

#include "../../scanner.hpp"
#include <windows.h>

#include <string>

#include <filesystem>
#include <fstream>


// Bridge functions for the windower module. These are used to expose core
// functionality to addons, and are not intended for use by addons directly.
namespace
{
extern "C" char const* get_package_list_ffi()
{
    static std::string result;
    result.clear();

    auto const& pm = windower::core::instance().package_manager;

    for (auto const& pkg : pm->installed_packages())
    {
        result.append(reinterpret_cast<char const*>(pkg->name().c_str()));
        result.append("|");
        result.append(
            reinterpret_cast<char const*>(
                windower::to_u8string(pkg->version()).c_str()));
        result.append("|");
        result.append(
            reinterpret_cast<char const*>(pkg->path().u8string().c_str()));
        result.append("|");

        // Check for the Readme securely in C++
        const bool has_readme =
            std::filesystem::exists(pkg->path() / u8"README.md");
        result.append(has_readme ? "1" : "0");
        result.append(";");
    }
    return result.c_str();
}

extern "C" char const* read_market_file_ffi(char const* filename)
{
    static std::string content;
    content.clear();

    // Call the standalone windower::user_path() function
    auto dir  = windower::user_path() / u8"addons" / u8"NextXIMarket";
    auto path = dir / reinterpret_cast<const char8_t*>(filename);

    std::ifstream file(path, std::ios::binary);
    if (file)
    {
        content.assign(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
    }
    return content.c_str();
}

extern "C" void write_market_file_ffi(char const* filename, char const* data)
{
    // Call the standalone windower::user_path() function
    auto dir = windower::user_path() / u8"addons" / u8"NextXIMarket";
    std::filesystem::create_directories(dir);

    auto path = dir / reinterpret_cast<const char8_t*>(filename);
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (file)
    {
        file << data;
    }
}

// New function to read the file securely
extern "C" char const* get_package_readme_ffi(char const* pkg_name)
{
    static std::string content;
    content.clear();

    std::u8string u8_name = reinterpret_cast<const char8_t*>(pkg_name);
    auto const& pm        = windower::core::instance().package_manager;
    auto pkg              = pm->get_package(u8_name);

    if (pkg)
    {
        auto readme_path = pkg->path() / u8"README.md";
        std::ifstream file(readme_path, std::ios::binary);
        if (file)
        {
            content.assign(
                (std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>());
        }
    }
    return content.c_str();
}

extern "C" char const* get_ffxi_player_ffi()
{
    // MOCK IMPLEMENTATION: In the future, this will read the player struct from FFXI memory
    static std::string player_json = 
        "{ \"name\": \"NextXIPlayer\", \"hp\": 1000, \"mp\": 500, \"tp\": 3000, "
        "\"main_job_id\": 1, \"main_job_level\": 99, \"sub_job_id\": 4, \"sub_job_level\": 49 }";
    return player_json.c_str();
}

extern "C" char const* get_ffxi_items_ffi()
{
    // MOCK IMPLEMENTATION: Will read inventory memory
    static std::string items_json = 
        "{ \"inventory\": [ { \"id\": 4100, \"count\": 1 }, { \"id\": 4101, \"count\": 99 } ], "
        "\"equipment\": { \"main\": 4100, \"sub\": 0 } }";
    return items_json.c_str();
}

extern "C" char const* get_ffxi_spells_ffi()
{
    // MOCK IMPLEMENTATION: Will read spells memory array
    static std::string spells_json = "[ 1, 2, 3, 4, 5 ]";
    return spells_json.c_str();
}

extern "C" char const* get_ffxi_entities_ffi()
{
    static std::string entities_json;
    static void** entity_array_ptr = nullptr;
    static bool scanned = false;

    if (!scanned)
    {
        if (::GetModuleHandleW(L"FFXiMain.dll"))
        {
            scanned = true;
            windower::signature sig{u8"8B560C8B042A8B0485"};
            auto results = windower::scan<1>(u8"FFXiMain.dll", sig);
            // Guard: only dereference if the signature scan returned a valid (non-null) address.
            // If results[0] is null (module not found or signature mismatch), (char*)null + 9
            // equals 0x9, which causes an access violation (crash at address 0x9).
            if (results[0])
            {
                void* match_addr = static_cast<void*>(results[0]);
                // The instruction is: mov eax, [eax*4 + XXXXXXXX]  (8B 04 85 XX XX XX XX)
                // The embedded 4-byte absolute address XXXXXXXX sits at byte offset 9 from
                // the start of our matched pattern (3 bytes for 8B56 0C + 3 bytes for 8B04 2A
                // + 3 bytes for 8B04 85 = 9 bytes before the address operand).
                entity_array_ptr = *reinterpret_cast<void***>(
                    static_cast<char*>(match_addr) + 9);
            }
        }
    }

    entities_json = "[";
    if (entity_array_ptr && !::IsBadReadPtr(entity_array_ptr, 2304 * sizeof(void*)))
    {
        bool first = true;
        for (int i = 0; i < 2304; ++i)
        {
            void* ent = entity_array_ptr[i];
            if (ent && !::IsBadReadPtr(ent, 0x0A0))
            {
                uint32_t id = *(uint32_t*)((char*)ent + 0x078);
                if (id == 0) continue;

                char name[24] = {0};
                memcpy(name, (char*)ent + 0x07C, 24);
                name[23] = '\0'; // ensure null termination
                if (strlen(name) == 0) continue;

                float x = *(float*)((char*)ent + 0x004);
                float z = *(float*)((char*)ent + 0x008);
                float y = *(float*)((char*)ent + 0x00C);

                if (!first) entities_json += ",";
                first = false;

                char buffer[256];
                snprintf(buffer, sizeof(buffer), "{\"id\":%u,\"name\":\"%s\",\"x\":%f,\"y\":%f,\"z\":%f,\"target_id\":0}", id, name, x, y, z);
                entities_json += buffer;
            }
        }
    }
    entities_json += "]";

    return entities_json.c_str();
}

extern "C" void project_ffi(float const* in_pos, float* out_screen)
{
    auto const& core = windower::core::instance();
    auto const& v = core.view_matrix;
    auto const& p = core.projection_matrix;
    auto const& vp = core.viewport;

    // View Transform
    float vx = in_pos[0]*v.m[0][0] + in_pos[1]*v.m[1][0] + in_pos[2]*v.m[2][0] + v.m[3][0];
    float vy = in_pos[0]*v.m[0][1] + in_pos[1]*v.m[1][1] + in_pos[2]*v.m[2][1] + v.m[3][1];
    float vz = in_pos[0]*v.m[0][2] + in_pos[1]*v.m[1][2] + in_pos[2]*v.m[2][2] + v.m[3][2];
    float vw = in_pos[0]*v.m[0][3] + in_pos[1]*v.m[1][3] + in_pos[2]*v.m[2][3] + v.m[3][3];

    // Projection Transform
    float cx = vx*p.m[0][0] + vy*p.m[1][0] + vz*p.m[2][0] + vw*p.m[3][0];
    float cy = vx*p.m[0][1] + vy*p.m[1][1] + vz*p.m[2][1] + vw*p.m[3][1];
    float cw = vx*p.m[0][3] + vy*p.m[1][3] + vz*p.m[2][3] + vw*p.m[3][3];

    if (cw < 0.1f) // Behind camera
    {
        out_screen[0] = -1.0f;
        out_screen[1] = -1.0f;
        return;
    }

    float ndcx = cx / cw;
    float ndcy = cy / cw;

    out_screen[0] = vp.X + (1.0f + ndcx) * vp.Width / 2.0f;
    out_screen[1] = vp.Y + (1.0f - ndcy) * vp.Height / 2.0f;
}
}

int windower::load_windower_module(lua::state s)
{
    lua::stack_guard guard{s};

    lua::load(guard, lua_windower_source, u8"core.windower");

    lua::push(guard, WINDOWER_VERSION_STRING); // version
    lua::push(guard, WINDOWER_VERSION_MAJOR); // version_major
    lua::push(guard, WINDOWER_VERSION_MINOR); // version_minor
    lua::push(guard, WINDOWER_VERSION_BUILD_STRING); // version_build
    lua::push(guard, WINDOWER_BUILD_TAG_STRING); // build_tag

    auto const scripts = windower_path() / u8"scripts";

    lua::push(guard, client_path().u8string()); // client_path
    lua::push(guard, scripts.u8string()); // scripts_path

    auto const& core = core::instance();

    lua::push(guard, core.settings.window_bounds.size.width); // client_width
    lua::push(guard, core.settings.window_bounds.size.height); // client_height
    lua::push(guard, core.settings.ui_size.width); // ui_width
    lua::push(guard, core.settings.ui_size.height); // ui_height

    lua::push(guard, core.client_hwnd); // client_hwnd

    if (auto const package = addon::get_package(s))
    {
        auto const settings = settings_path() / u8"settings" / package->name();
        auto const user     = user_path() / u8"addons" / package->name();

        lua::push(guard, settings.u8string()); // settings_path
        lua::push(guard, user.u8string()); // user_path
        lua::push(guard, package->path().u8string()); // package_path
        lua::push(guard, package->name()); // package_name
    }
    else
    {
        lua::push(guard, lua::nil); // settings_path
        lua::push(guard, lua::nil); // user_path
        lua::push(guard, lua::nil); // package_path
        lua::push(guard, lua::nil); // package_name
    }

    // Push the get_package_list function as a closure, so it can be called from
    // Lua to retrieve the list of installed packages.
    lua::push(guard, &get_package_list_ffi); // <--- Use the FFI safe pointer
    lua::push(guard, &get_package_readme_ffi);
    lua::push(guard, &read_market_file_ffi); 
    lua::push(guard, &write_market_file_ffi); 
    
    lua::push(guard, &get_ffxi_player_ffi);
    lua::push(guard, &get_ffxi_items_ffi);
    lua::push(guard, &get_ffxi_spells_ffi);
    lua::push(guard, &get_ffxi_entities_ffi);
    lua::push(guard, &project_ffi);

    // The windower module expects 25 upvalues, which are the values we just pushed
    lua::call(guard, 25);

    return guard.release();
}
