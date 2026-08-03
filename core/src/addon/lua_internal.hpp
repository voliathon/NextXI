#ifndef WINDOWER_ADDON_LUA_INTERNAL_HPP
#define WINDOWER_ADDON_LUA_INTERNAL_HPP

#include "addon/lua.hpp"

#include <cstddef>
#include <memory>

namespace windower::lua
{

class interpreter
{
public:
    interpreter() noexcept;
    interpreter(interpreter&&) noexcept;
    interpreter(interpreter const&) = delete;

    ~interpreter();

    interpreter& operator=(interpreter&&) noexcept;
    interpreter& operator=(interpreter const&) = delete;

    operator state() const noexcept;

private:
    state m_state = {};
};

enum class lib
{
    math,
    string,
    table,
    io,
    os,
    package,
    debug,
    bit,
    jit,
    ffi,
};

void load(interpreter const&, lib);
void load(interpreter&&, lib) = delete;

void preload(interpreter const&, lib);
void preload(interpreter&&, lib) = delete;
void preload(
    interpreter const&, std::u8string_view, std::function<int(state)> const&);
void preload(
    interpreter&&, std::u8string_view,
    std::function<int(state)> const&) = delete;

}

#endif