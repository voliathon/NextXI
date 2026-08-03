#include "addon/modules/scanner.hpp"

#include "../../scanner.hpp"
#include "addon/lua.hpp"
#include "addon/modules/scanner.lua.hpp"

extern "C"
{
    static void* scan_native(
        char8_t const* module_string, std::size_t module_length,
        char8_t const* signature_string, std::size_t signature_length)
    {
        return windower::scan(
            {module_string, module_length},
            windower::signature{{signature_string, signature_length}});
    }
}

int windower::load_scanner_module(lua::state s)
{
    lua::stack_guard guard{s};

    lua::load(guard, lua_scanner_source, u8"core.scanner");

    lua::push(guard, &scan_native);
    lua::call(guard, 1);

    return guard.release();
}