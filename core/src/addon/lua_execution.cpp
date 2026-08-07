#include "addon/lua.hpp"
#include "addon/error.hpp"
#include "addon/unsafe.hpp"
#include <lua.hpp>
#include <functional>

namespace
{
    std::byte std_function_metatable_key;
}

extern "C"
{
    static int destroy_std_function(::lua_State* s)
    {
        if (::lua_getmetatable(s, 1))
        {
            ::lua_pushlightuserdata(s, &std_function_metatable_key);
            ::lua_rawget(s, LUA_REGISTRYINDEX);
            auto const result = ::lua_rawequal(s, -2, -1);
            ::lua_settop(s, -3);
            if (result)
            {
                if (auto ptr = ::lua_touserdata(s, 1))
                {
                    ::lua_pushnil(s);
                    ::lua_setmetatable(s, 1);
                    static_cast<std::function<int(windower::lua::state)>*>(ptr)->~function();
                    return 0;
                }
            }
        }
        ::lua_pushlstring(s, "[INTERNAL ERROR] invalid state", 30);
        return ::lua_error(s);
    }

    static int call_std_function(::lua_State* s)
    {
        try
        {
            if (::lua_getmetatable(s, windower::lua::upvalue(0)))
            {
                ::lua_pushlightuserdata(s, &std_function_metatable_key);
                ::lua_rawget(s, LUA_REGISTRYINDEX);
                auto const result = ::lua_rawequal(s, -2, -1);
                ::lua_settop(s, -3);
                if (result)
                {
                    if (auto ptr = ::lua_touserdata(s, windower::lua::upvalue(0)))
                        return (*static_cast<std::function<int(windower::lua::state)>*>(ptr))(windower::lua::unsafe::wrap(s));
                }
            }
            ::lua_pushlstring(s, "[INTERNAL ERROR] invalid state", 30);
        }
        catch (windower::lua::error const& e)
        {
            if (e.has_stack_trace()) windower::lua::unsafe::set_stack_trace(s, e.stack_trace());
            else windower::lua::unsafe::set_stack_trace(s);
            ::lua_pushstring(s, e.what());
        }
        return ::lua_error(s);
    }

    static char const* reader_impl(::lua_State*, void* context, std::size_t* size)
    {
        auto const buffer = static_cast<windower::lua::reader*>(context)->read();
        *size = buffer.size();
        GSL_SUPPRESS("type.1") { return reinterpret_cast<char const*>(buffer.data()); }
    }

    static int writer_impl(::lua_State*, void const* data, std::size_t size, void* context)
    {
        static_cast<windower::lua::writer*>(context)->write({ static_cast<std::byte const*>(data), size });
        return 0;
    }
}

void windower::lua::push(stack_guard const& s, std::function<int(state)> const& value, std::size_t upvalues)
{
    if (s.size() < upvalues) throw error{ "too few stack arguments" };

    auto ptr = windower::lua::create_userdata(s, sizeof(std::function<int(state)>));
    push(s, &std_function_metatable_key);
    raw_get(s, registry);
    if (typeof(s, -1) != type::table)
    {
        pop(s);
        create_table(s, 0, 1);
        push(s, u8"__gc");
        ::lua_pushcclosure(unsafe::unwrap(s), ::destroy_std_function, 0);
        raw_set(s, -3);
        push(s, u8"__metatable");
        push(s, nil);
        raw_set(s, -3);
        push(s, &std_function_metatable_key);
        copy(s, -2);
        raw_set(s, registry);
    }
    set_metatable(s, -2);
    try { [[gsl::suppress("r.11")]] new (ptr) std::function<int(state)>{value}; }
    catch (...) { push(s, nil); set_metatable(s, -2); throw; }

    if (upvalues) insert(s, -gsl::narrow<int>(upvalues + 1));
    ::lua_pushcclosure(unsafe::unwrap(s), ::call_std_function, upvalues + 1);
}

void windower::lua::call(stack_guard const& s, std::size_t args, std::size_t results)
{
    if (s.size() < args) throw error{ "too few stack arguments" };

    auto unwrapped = unsafe::unwrap(s);
    auto const top = ::lua_gettop(unwrapped) - args;
    ::lua_pushlightuserdata(unwrapped, windower::lua::unsafe::key::error_handler);
    ::lua_rawget(unwrapped, LUA_REGISTRYINDEX);
    ::lua_insert(unwrapped, top);
    auto const result = ::lua_pcall(unwrapped, args, results, top);
    ::lua_remove(unwrapped, top);
    switch (result)
    {
    case LUA_OK: return;
    case LUA_ERRRUN:
    case LUA_ERRMEM:
    case LUA_ERRERR: throw windower::lua::error{ s };
    default: throw error{ "[INTERNAL ERROR] invalid state", s };
    }
}

bool windower::lua::resume(stack_guard const& s, std::size_t args)
{
    if (s.size() < args) throw error{ "too few stack arguments" };
    switch (::lua_resume(unsafe::unwrap(s), args))
    {
    case LUA_OK: return false;
    case LUA_YIELD: return true;
    case LUA_ERRRUN:
    case LUA_ERRMEM:
    case LUA_ERRERR:
        push(s, unsafe::key::error_handler);
        raw_get(s, registry);
        insert(s, -2);
        call(s, 1);
        throw error{ s };
    default: throw error{ "[INTERNAL ERROR] invalid state", s };
    }
}

void windower::lua::load(stack_guard const& s, reader& r, u8zstring_view name)
{
    if (::lua_load(unsafe::unwrap(s), ::reader_impl, &r, to_zstring_view(name).c_str()))
        throw lua::error{ s };
}

void windower::lua::load(stack_guard const& s, std::span<std::byte const> buffer, u8zstring_view name)
{
    class buffer_reader final : public reader
    {
    public:
        buffer_reader(std::span<std::byte const> buffer) noexcept : m_buffer{ buffer } {}
        std::span<std::byte const> read() noexcept override
        {
            auto result = m_buffer;
            if (!m_done) { m_buffer = {}; m_done = true; }
            return result;
        }
    private:
        std::span<std::byte const> m_buffer;
        bool m_done = false;
    };
    buffer_reader r{ buffer };
    load(s, r, name);
}

void windower::lua::load(stack_guard const& s, std::u8string_view source, u8zstring_view name) { load(s, std::as_bytes(std::span<char8_t const>{source}), name); }
void windower::lua::load(stack_guard const& s, std::u8string_view source) { load(s, source, std::u8string{ source }.c_str()); }

void windower::lua::load(stack_guard const& s, std::istream& stream, u8zstring_view name)
{
    class stream_reader final : public reader
    {
    public:
        stream_reader(std::istream& stream) noexcept : m_stream{ stream } {}
        std::span<std::byte const> read() noexcept override
        {
            if (!m_stream) return {};
            GSL_SUPPRESS("type.1")
            {
                m_stream.read(reinterpret_cast<char*>(m_buffer.data()), m_buffer.size());
            }
            return std::span{ m_buffer }.subspan(0, gsl::narrow_cast<std::size_t>(m_stream.gcount()));
        }
    private:
        std::array<std::byte, 0x2000> m_buffer = {};
        std::istream& m_stream;
    };
    stream_reader r{ stream };
    load(s, r, name);
}

void windower::lua::save(state s, writer& w) { ::lua_dump(unsafe::unwrap(s), ::writer_impl, &w); }

void windower::lua::save(state s, std::ostream& stream)
{
    class stream_writer sealed : public writer
    {
    public:
        stream_writer(std::ostream& stream) noexcept : m_stream{ stream } {}
        void write(std::span<std::byte const> buffer) noexcept override
        {
            if (m_stream) m_stream.write(reinterpret_cast<char const*>(buffer.data()), buffer.size());
        }
    private:
        std::ostream& m_stream;
    };
    stream_writer w{ stream };
    save(s, w);
}
