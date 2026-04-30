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
    auto dir  = windower::user_path() / u8"addons" / u8"FenestraMarket";
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
    auto dir = windower::user_path() / u8"addons" / u8"FenestraMarket";
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

    auto const scripts = user_path() / u8"scripts";

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

    // The windower module expects 20 upvalues, which are the values we just pushed
    lua::call(guard, 20);

    return guard.release();
}