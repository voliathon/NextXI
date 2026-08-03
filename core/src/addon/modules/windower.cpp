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

#include <string>
#include <span>

#include <filesystem>
#include <fstream>
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
    static std::string player_json = 
        "{ \"name\": \"NextXIPlayer\", \"hp\": 1000, \"mp\": 500, \"tp\": 3000, "
        "\"main_job_id\": 1, \"main_job_level\": 99, \"sub_job_id\": 4, \"sub_job_level\": 49 }";
    return player_json.c_str();
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

struct entity_slot_t
{
    uint32_t id;
    char     name[24];
    float    x, y, z;
};

// Fix: Added noexcept, null check, and proper reinterpret_casts.
// Fix: Restricted SEH to only catch Access Violations to avoid masking deeper issues.
static void** seh_resolve_entity_ptr(void* match_addr) noexcept
{
    if (!match_addr)
    {
        return nullptr;
    }

    void** out = nullptr;
    __try
    {
        out = *reinterpret_cast<void***>(reinterpret_cast<std::byte*>(match_addr) + 9);
    }
    __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        out = nullptr;
    }
    return out;
}

// Fix: Added noexcept, null checks, proper C++ casting, span for arrays, and const bounds.
static bool seh_read_entity_slot(void** arr, int idx, entity_slot_t* out) noexcept
{
    if (!arr || !out)
    {
        return false;
    }

    __try
    {
        // Fix: Use span to avoid pointer arithmetic warnings on the array
        std::span<void*> const arr_span{ arr, 2304 };
        void* ent = arr_span[idx];
        if (!ent) return false;

        auto* const byte_ent = reinterpret_cast<std::byte*>(ent);

        out->id = *reinterpret_cast<uint32_t*>(byte_ent + 0x078);
        if (out->id == 0) return false;

        // Fix: Span wraps the C-array to prevent decay warnings
        std::span<char, 24> name_span{ out->name };
        std::fill(name_span.begin(), name_span.end(), '\0');
        std::memcpy(name_span.data(), byte_ent + 0x07C, 23);

        if (name_span[0] == '\0') return false;

        for (std::size_t j = 0; j < 23 && name_span[j] != '\0'; ++j)
        {
            auto const ch = static_cast<unsigned char>(name_span[j]);
            if (ch < 0x20 || ch == '"' || ch == '\\')
            {
                name_span[j] = '?';
            }
        }

        out->x = *reinterpret_cast<float*>(byte_ent + 0x004);
        out->z = *reinterpret_cast<float*>(byte_ent + 0x008);
        out->y = *reinterpret_cast<float*>(byte_ent + 0x00C);

        if (out->x != out->x || out->y != out->y || out->z != out->z) return false;

        return true;
    }
    __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        return false;
    }
}

extern "C" char const* get_ffxi_entities_ffi()
{
    static std::string entities_json;
    static void** entity_array_ptr = nullptr;
    static bool        scanned = false;

    if (!scanned && ::GetModuleHandleW(L"FFXiMain.dll"))
    {
        scanned = true;
        windower::signature const sig{ u8"8B560C8B042A8B0485" }; // Fix: Added const
        auto results = windower::scan<1>(u8"FFXiMain.dll", sig);
        if (results[0])
        {
            entity_array_ptr = seh_resolve_entity_ptr(static_cast<void*>(results[0]));
            if (!entity_array_ptr) scanned = false;
        }
    }

    entities_json = "[";

    if (entity_array_ptr)
    {
        bool first = true;
        entity_slot_t slot;

        for (int i = 0; i < 2304; ++i)
        {
            if (!seh_read_entity_slot(entity_array_ptr, i, &slot)) continue;

            if (!first) entities_json += ",";
            first = false;

            char buffer[256];
            std::snprintf(buffer, sizeof(buffer),
                "{\"id\":%u,\"name\":\"%s\",\"x\":%.2f,\"y\":%.2f,\"z\":%.2f,\"target_id\":0}",
                slot.id, slot.name, static_cast<double>(slot.x), static_cast<double>(slot.y), static_cast<double>(slot.z));
            entities_json += buffer;
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

    // Using span to safely read the FFI float arrays without bounds/decay warnings
    std::span<float const, 3> const in_span{ in_pos, 3 };
    std::span<float, 2> const out_span{ out_screen, 2 };

    // Added const to all math variables
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
