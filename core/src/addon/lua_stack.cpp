#include "addon/lua.hpp"
#include "addon/error.hpp"
#include "addon/unsafe.hpp"
#include <lua.hpp>

windower::lua::stack_guard::stack_guard(state s) :
    m_state{ s }, m_base{ gsl::narrow_cast<std::size_t>(::lua_gettop(unsafe::unwrap(s))) } {
}

windower::lua::stack_guard::stack_guard(stack_guard& s) :
    m_state{ s.m_state }, m_base{ gsl::narrow_cast<std::size_t>(::lua_gettop(unsafe::unwrap(s))) } {
}

windower::lua::stack_guard::~stack_guard()
{
    if (auto unwrapped = unsafe::unwrap(m_state); unwrapped && ::lua_gettop(unwrapped) > gsl::narrow_cast<int>(m_base))
        ::lua_settop(unwrapped, m_base);
}

windower::lua::stack_guard::operator windower::lua::state() const noexcept { return m_state; }
std::size_t windower::lua::stack_guard::base() const noexcept { return m_base; }
std::size_t windower::lua::stack_guard::size() const noexcept { return ::lua_gettop(unsafe::unwrap(m_state)) - m_base; }

std::size_t windower::lua::stack_guard::release() noexcept
{
    auto const count = size();
    unsafe::unwrap(m_state) = nullptr;
    m_base = 0;
    return count;
}

int windower::lua::absolute(state s, int index) noexcept { return unsafe::absolute(unsafe::unwrap(s), index); }

void windower::lua::top(stack_guard const& s, int top)
{
    auto absolute = unsafe::absolute(unsafe::unwrap(s), top);
    if (absolute > 0)
    {
        if (gsl::narrow_cast<std::size_t>(absolute) < s.base()) absolute = s.base();
        ::lua_settop(unsafe::unwrap(s), absolute);
    }
}

void windower::lua::pop(stack_guard const& s, std::size_t count)
{
    auto new_top = ::lua_gettop(unsafe::unwrap(s)) - count;
    if (new_top < s.base()) new_top = s.base();
    ::lua_settop(unsafe::unwrap(s), new_top);
}

void windower::lua::copy(stack_guard const& s, int index) { ::lua_pushvalue(unsafe::unwrap(s), index); }
void windower::lua::push(stack_guard const& s, nil_t) { ::lua_pushnil(unsafe::unwrap(s)); }
void windower::lua::push(stack_guard const& s, bool value) { ::lua_pushboolean(unsafe::unwrap(s), value); }
void windower::lua::push(stack_guard const& s, void* value) { ::lua_pushlightuserdata(unsafe::unwrap(s), value); }
void windower::lua::push(stack_guard const& s, double value) { ::lua_pushnumber(unsafe::unwrap(s), value); }
void windower::lua::push(stack_guard const& s, std::int32_t value) { ::lua_pushinteger(unsafe::unwrap(s), value); }

void windower::lua::push(stack_guard const& s, char8_t const* value)
{
    auto const view = to_string_view(value);
    ::lua_pushlstring(unsafe::unwrap(s), view.data(), view.size());
}

void windower::lua::push(stack_guard const& s, std::u8string_view value)
{
    auto const view = to_string_view(value);
    ::lua_pushlstring(unsafe::unwrap(s), view.data(), view.size());
}

void windower::lua::push(stack_guard const& s, std::span<std::byte const> value)
{
    GSL_SUPPRESS("type.1")
    {
        auto const ptr = reinterpret_cast<char const*>(value.data());
        ::lua_pushlstring(unsafe::unwrap(s), ptr, value.size());
    }
}

bool windower::lua::push(stack_guard const& s, state value)
{
    auto const result = ::lua_pushthread(unsafe::unwrap(value)) != 0;
    if (unsafe::unwrap(s) != unsafe::unwrap(value))
        ::lua_xmove(unsafe::unwrap(value), unsafe::unwrap(s), 1);
    return result;
}

bool windower::lua::push(stack_guard const& s, stack_guard const& value) { return push(s, state{ value }); }
void windower::lua::create_table(stack_guard const& s, std::size_t array_size, std::size_t hash_size) { ::lua_createtable(unsafe::unwrap(s), array_size, hash_size); }
void* windower::lua::create_userdata(stack_guard const& s, std::size_t size) { return ::lua_newuserdata(unsafe::unwrap(s), size); }
windower::lua::state windower::lua::create_coroutine(stack_guard const& s) { return unsafe::wrap(::lua_newthread(unsafe::unwrap(s))); }

void windower::lua::get(stack_guard const& s, int index) { ::lua_gettable(unsafe::unwrap(s), index); }
void windower::lua::set(stack_guard const& s, int index) { ::lua_settable(unsafe::unwrap(s), index); }
void windower::lua::get(stack_guard const& s, int index, u8zstring_view key) { ::lua_getfield(unsafe::unwrap(s), index, to_zstring_view(key).data()); }
void windower::lua::set(stack_guard const& s, int index, u8zstring_view key) { ::lua_setfield(unsafe::unwrap(s), index, to_zstring_view(key).data()); }
void windower::lua::raw_get(stack_guard const& s, int index) { ::lua_rawget(unsafe::unwrap(s), index); }
void windower::lua::raw_set(stack_guard const& s, int index) { ::lua_rawset(unsafe::unwrap(s), index); }
void windower::lua::raw_get(stack_guard const& s, int index, int key) { ::lua_rawgeti(unsafe::unwrap(s), index, key); }
void windower::lua::raw_set(stack_guard const& s, int index, int key) { ::lua_rawseti(unsafe::unwrap(s), index, key); }
bool windower::lua::get_metatable(stack_guard const& s, int index) { return ::lua_getmetatable(unsafe::unwrap(s), index) != 0; }
void windower::lua::set_metatable(stack_guard const& s, int index) { ::lua_setmetatable(unsafe::unwrap(s), index); }
void windower::lua::get_environment(stack_guard const& s, int index) { ::lua_getfenv(unsafe::unwrap(s), index); }
void windower::lua::set_environment(stack_guard const& s, int index) { ::lua_setfenv(unsafe::unwrap(s), index); }
void windower::lua::insert(stack_guard const& s, int index) { ::lua_insert(unsafe::unwrap(s), index); }
void windower::lua::remove(stack_guard const& s, int index) { ::lua_remove(unsafe::unwrap(s), index); }
void windower::lua::replace(stack_guard const& s, int index) { ::lua_replace(unsafe::unwrap(s), index); }
void windower::lua::concat(stack_guard const& s, std::size_t count) { ::lua_concat(unsafe::unwrap(s), count); }
void windower::lua::xmove(stack_guard const& src, stack_guard const& dst, std::size_t count) { ::lua_xmove(unsafe::unwrap(src), unsafe::unwrap(dst), count); }
