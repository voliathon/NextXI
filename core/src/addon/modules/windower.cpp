#include "windower.hpp"

#include "addon/addon.hpp"
#include "addon/package_manager.hpp"
#include "addon/lua.hpp"
#include "addon/modules/windower.lua.hpp"
#include "core.hpp"
#include "utility.hpp"
#include "utilities/paths.hpp"
#include "version.hpp"

#include "../../scanner.hpp"
#include <windows.h>
#include <cstring>

#include <string>
#include <span>

#include <filesystem>
#include <fstream>
#include <chrono>
#include "hooks/user32_internal.hpp"

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
        auto dir = windower::user_path() / u8"addons" / u8"NextXIMarket";
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
        auto dir = windower::user_path() / u8"addons" / u8"NextXIMarket";
        std::filesystem::create_directories(dir);

        auto path = dir / reinterpret_cast<const char8_t*>(filename);
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (file)
        {
            file << data;
        }
    }
    extern "C" char const* get_package_readme_ffi(char const* pkg_name)
    {
        static std::string content;
        content.clear();

        std::u8string u8_name = reinterpret_cast<const char8_t*>(pkg_name);
        auto const& pm = windower::core::instance().package_manager;
        auto pkg = pm->get_package(u8_name);

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

    struct entity_slot_t
    {
        uint32_t id;
        char     name[24];
        float    x, y, z;
    };

#pragma pack(push, 1)
    struct ffxi_entity_memory
    {
        std::byte padding1[0x04];
        float x;
        float z;
        float y;
        std::byte padding2[0x078 - 0x010];
        uint32_t id;
        char name[24];
    };
#pragma pack(pop)

    // --- DYNAMIC MEMORY SCANNER & PLAYER EXTRACTION ---
    static void** g_entity_array_ptr = nullptr;
    static bool g_scanned_entities = false;

#pragma warning(push)
#pragma warning(disable: 6320 26429 26446 26462 26471 26472 26481 26482 26485 26493 26496)

    // The crash-proof VirtualQuery scanner
    static void** safe_scan_ffximain() noexcept {
        HMODULE hMod = ::GetModuleHandleW(L"FFXiMain.dll");
        if (!hMod) return nullptr;

        uint8_t* base = reinterpret_cast<uint8_t*>(hMod);
        PIMAGE_DOS_HEADER dos = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
        PIMAGE_NT_HEADERS nt = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos->e_lfanew);
        DWORD size = nt->OptionalHeader.SizeOfImage;

        MEMORY_BASIC_INFORMATION mbi;
        for (uint8_t* curr = base; curr < base + size; curr += mbi.RegionSize) {
            if (!::VirtualQuery(curr, &mbi, sizeof(mbi))) break;

            // Only scan committed, readable memory. Skip guard pages entirely.
            if (mbi.State != MEM_COMMIT) continue;
            if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) continue;

            uint8_t* region_end = static_cast<uint8_t*>(mbi.BaseAddress) + mbi.RegionSize;
            uint8_t* p = static_cast<uint8_t*>(mbi.BaseAddress);

            for (; p < region_end - 9; ++p) {
                // Hardcoded fast-check for your confirmed JSON signature
                if (p[0] == 0x8B && p[1] == 0x56 && p[2] == 0x0C && p[3] == 0x8B &&
                    p[4] == 0x04 && p[5] == 0x2A && p[6] == 0x8B && p[7] == 0x04 && p[8] == 0x85) {

                    void** out = nullptr;
                    std::memcpy(&out, p + 9, sizeof(void**));
                    return out;
                }
            }
        }
        return nullptr;
    }

    static void scan_entities_if_needed()
    {
        if (!g_scanned_entities && ::GetModuleHandleW(L"FFXiMain.dll"))
        {
            static auto last_scan = std::chrono::steady_clock::time_point::min();
            if (std::chrono::steady_clock::now() - last_scan < std::chrono::seconds(2)) return;
            last_scan = std::chrono::steady_clock::now();

            void** array_ptr = safe_scan_ffximain();

            if (array_ptr && array_ptr != (void**)0x0) {
                g_entity_array_ptr = array_ptr;
                g_scanned_entities = true;
            }
        }
    }

    // Safely rip exactly 24 bytes from offset 0x7C
    static bool safe_read_entity_name(void** arr, int idx, char* out_name) noexcept {
        __try {
            void* ent = arr[idx];
            if (!ent) return false;

            std::memcpy(out_name, reinterpret_cast<char*>(ent) + 0x7C, 24);
            return true;
        }
        __except (1) {
            return false;
        }
    }

    extern "C" char const* get_ffxi_player_ffi()
    {
        scan_entities_if_needed();

        if (!g_scanned_entities || !g_entity_array_ptr) return nullptr;

        // Scan strictly inside the Player Character block (1024 - 2304)
        for (int i = 1024; i < 2304; ++i)
        {
            char name[28] = { 0 };

            if (safe_read_entity_name(g_entity_array_ptr, i, name))
            {
                // If the name starts with a valid capital letter, we found Voreus!
                if (name[0] >= 'A' && name[0] <= 'Z')
                {
                    // Force null termination just to be safe
                    name[24] = '\0';

                    static std::string player_json;
                    player_json = "{ \"name\": \"";
                    player_json += name;
                    player_json += "\", \"hp\": 1000, \"mp\": 500, \"tp\": 3000, "
                        "\"main_job_id\": 1, \"main_job_level\": 99, \"sub_job_id\": 4, \"sub_job_level\": 49 }";

                    return player_json.c_str();
                }
            }
        }

        return nullptr;
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
#pragma warning(pop)


    // Use MSVC pragmas to bypass the extern "C" parsing bug
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

        if (cw < 0.1f) // Behind camera
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

int windower::load_windower_module(lua::state s)
{
    lua::stack_guard guard{ s };

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
        auto const user = user_path() / u8"addons" / package->name();

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
    lua::push(guard, &get_package_list_ffi); // <--- Use the FFI safe pointer
    lua::push(guard, &get_package_readme_ffi);
    lua::push(guard, &read_market_file_ffi);
    lua::push(guard, &write_market_file_ffi);

    lua::push(guard, &get_ffxi_player_ffi);
    lua::push(guard, &get_ffxi_items_ffi);
    lua::push(guard, &get_ffxi_spells_ffi);
    lua::push(guard, &get_ffxi_entities_ffi);
    lua::push(guard, &project_ffi);
    lua::call(guard, 25);

    return guard.release();
}
