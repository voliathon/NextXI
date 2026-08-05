#ifndef WINDOWER_ADDON_UNSAFE_HPP
#define WINDOWER_ADDON_UNSAFE_HPP
#pragma once

#include "error.hpp"
#include "lua.hpp"
#include "utility.hpp"

#include <memory>
#include <type_traits>
#include <typeinfo>
#include <vector>

#if defined(_MSC_VER)
#    define WINDOWER_NOINLINE __declspec(noinline)
#elif defined(__GNUC__)
#    define WINDOWER_NOINLINE __attribute__((noinline))
#else
#    error "WINDOWER_NOINLINE macro is not defined for this compiler"
#endif

extern "C"
{
    struct lua_State;
}

namespace windower::lua
{

class stack_frame;

namespace unsafe
{

::lua_State*& unwrap(state&) noexcept;
::lua_State* const& unwrap(state const&) noexcept;
::lua_State* unwrap(stack_guard const&) noexcept;
state wrap(::lua_State*) noexcept;
int absolute(::lua_State*, int) noexcept;
void set_base(stack_guard&, std::size_t) noexcept;
void set_stack_trace(::lua_State*);
void set_stack_trace(::lua_State*, std::vector<stack_frame> const&);
std::unique_ptr<std::vector<stack_frame> const> get_stack_trace(::lua_State*);

namespace key
{
extern void* const error_handler;
extern void* const cache;
extern void* const to_string;
extern void* const stack_trace;
}

}

}

#endif