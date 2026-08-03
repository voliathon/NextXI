#ifndef WINDOWER_ADDON_ERROR_HPP
#define WINDOWER_ADDON_ERROR_HPP
#pragma once

#include "addon/lua.hpp"

#include <memory>
#include <stdexcept>
#include <vector>

namespace windower::lua
{

class source_descriptor
{
public:
    std::u8string type;
    std::u8string value;
    std::size_t line = 0;
};

class variable_descriptor
{
public:
    std::u8string name;
    std::u8string type;
    std::u8string value;
};

class stack_frame
{
public:
    std::u8string name;
    std::u8string type;
    source_descriptor source;
    std::vector<variable_descriptor> locals;
    std::vector<variable_descriptor> upvalues;
};

class error : public std::runtime_error
{
public:
    error(state);
    error(std::string const&, state);
    error(char const*, state);
    error(std::string const&);
    error(char const*);

    bool has_stack_trace() const noexcept;
    std::vector<stack_frame> const& stack_trace() const noexcept;

private:
    std::shared_ptr<std::vector<stack_frame> const> m_stack_trace;
};

}

#endif