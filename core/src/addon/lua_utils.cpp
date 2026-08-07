#include "addon/lua.hpp"
#include "addon/error.hpp"
#include "addon/unsafe.hpp"
#include <lua.hpp>

namespace
{
    void throw_argument_error(windower::lua::state s, std::size_t index)
    {
        using namespace windower;
        ::lua_Debug info;
        if (::lua_getstack(lua::unsafe::unwrap(s), 0, &info) && ::lua_getinfo(lua::unsafe::unwrap(s), "n", &info))
            throw lua::error{ "bad argument #" + std::to_string(index) + " to '" + info.name + "' (value expected)" };
        throw lua::error{ "bad argument #" + std::to_string(index) + " (value expected)" };
    }

    void throw_argument_type_error(windower::lua::state s, std::size_t index, windower::lua::type expected, windower::lua::type actual)
    {
        using namespace windower;
        ::lua_Debug info;
        auto message = std::u8string{};
        message.append(u8"bad argument #").append(to_u8string(index));
        if (::lua_getstack(lua::unsafe::unwrap(s), 0, &info) && ::lua_getinfo(lua::unsafe::unwrap(s), "n", &info))
            message.append(u8" to '").append(to_u8string(info.name)).append(u8"' (");
        else
            message.append(u8" (");

        message.append(to_u8string_view(expected)).append(u8" expected, got ").append(to_u8string_view(actual)).append(u8")");
        throw lua::error{ windower::to_string(message) };
    }
}

std::u8string_view windower::lua::to_u8string_view(type value)
{
    switch (value)
    {
    case type::none: return u8"no value";
    case type::nil: return u8"nil";
    case type::boolean: return u8"boolean";
    case type::lightuserdata: return u8"userdata";
    case type::number: return u8"number";
    case type::string: return u8"string";
    case type::table: return u8"table";
    case type::function: return u8"function";
    case type::userdata: return u8"userdata";
    case type::coroutine: return u8"thread";
    case type::cdata: return u8"cdata";
    }
    throw error{ "[INTERNAL ERROR] unknown lua type" };
}

std::u8string windower::lua::to_u8string(type value) { return std::u8string{ to_u8string_view(value) }; }

void windower::lua::reserve(state s, std::size_t size)
{
    if (!::lua_checkstack(unsafe::unwrap(s), size)) throw error{ "stack overflow", s };
}

std::size_t windower::lua::top(state s) { return ::lua_gettop(unsafe::unwrap(s)); }
windower::lua::type windower::lua::typeof(state s, int index) { return static_cast<type>(::lua_type(unsafe::unwrap(s), index)); }

void windower::lua::check_argument(state s, std::size_t index)
{
    if (typeof(s, index) == type::none) throw_argument_error(s, index);
}

void windower::lua::check_argument(state s, std::size_t index, type expected)
{
    auto const argument_type = typeof(s, index);
    if (argument_type != expected) throw_argument_type_error(s, index, expected, argument_type);
}

bool windower::lua::check_optional_argument(state s, std::size_t index, type expected)
{
    auto const argument_type = typeof(s, index);
    if (argument_type == type::nil || argument_type == type::none) return false;
    if (argument_type != expected) throw_argument_type_error(s, index, expected, argument_type);
    return true;
}

std::size_t windower::lua::size(state s, int index) { return ::lua_objlen(unsafe::unwrap(s), index); }
bool windower::lua::equal(state s, int index1, int index2) { return ::lua_equal(unsafe::unwrap(s), index1, index2) != 0; }
bool windower::lua::raw_equal(state s, int index1, int index2) { return ::lua_rawequal(unsafe::unwrap(s), index1, index2) != 0; }
bool windower::lua::less(state s, int index1, int index2) { return ::lua_lessthan(unsafe::unwrap(s), index1, index2) != 0; }
windower::lua::coroutine_status windower::lua::status(state s) { return static_cast<coroutine_status>(::lua_status(unsafe::unwrap(s))); }
bool windower::lua::next(stack_guard const& s, int index) { return ::lua_next(unsafe::unwrap(s), index) != 0; }

std::size_t windower::lua::memory_usage(state s)
{
    return gsl::narrow_cast<std::size_t>(
        ::lua_gc(unsafe::unwrap(s), LUA_GCCOUNT, 0) * 1024 +
        ::lua_gc(unsafe::unwrap(s), LUA_GCCOUNTB, 0));
}

void windower::lua::gc_start(state s) { ::lua_gc(unsafe::unwrap(s), LUA_GCRESTART, 0); }
void windower::lua::gc_stop(state s) { ::lua_gc(unsafe::unwrap(s), LUA_GCSTOP, 0); }
void windower::lua::gc_configure(state s, float pause, float step_multiplier)
{
    ::lua_gc(unsafe::unwrap(s), LUA_GCSETPAUSE, gsl::narrow_cast<int>(std::round(pause * 100)));
    ::lua_gc(unsafe::unwrap(s), LUA_GCSETSTEPMUL, gsl::narrow_cast<int>(std::round(step_multiplier * 100)));
}
void windower::lua::gc_collect(state s) { ::lua_gc(unsafe::unwrap(s), LUA_GCCOLLECT, 0); }
void windower::lua::gc_increment(state s, std::size_t step) { ::lua_gc(unsafe::unwrap(s), LUA_GCSTEP, step); }

bool windower::lua::detail::get_bool(state s, int index) { return ::lua_toboolean(unsafe::unwrap(s), index) != 0; }
double windower::lua::detail::get_number(state s, int index) { return ::lua_tonumber(unsafe::unwrap(s), index); }
std::ptrdiff_t windower::lua::detail::get_integer(state s, int index) { return ::lua_tointeger(unsafe::unwrap(s), index); }
void* windower::lua::detail::get_userdata(state s, int index) { return ::lua_touserdata(unsafe::unwrap(s), index); }
void const* windower::lua::detail::get_pointer(state s, int index) { return ::lua_topointer(unsafe::unwrap(s), index); }
windower::lua::state windower::lua::detail::get_coroutine(state s, int index) { return unsafe::wrap(::lua_tothread(unsafe::unwrap(s), index)); }

std::u8string_view windower::lua::detail::get_string_view(state s, int index)
{
    if (typeof(s, index) == type::string)
    {
        std::size_t size;
        if (auto ptr = ::lua_tolstring(unsafe::unwrap(s), index, &size))
        {
            GSL_SUPPRESS("type.1") { return { reinterpret_cast<char8_t const*>(ptr), size }; }
        }
    }
    return {};
}

std::span<std::byte const> windower::lua::detail::get_data_string_span(state s, int index)
{
    if (typeof(s, index) == type::string)
    {
        std::size_t size;
        if (auto ptr = ::lua_tolstring(unsafe::unwrap(s), index, &size))
            return std::as_bytes(std::span{ ptr, size });
    }
    return {};
}
