#ifndef WINDOWER_HOOKLIB_HOOK_HPP
#define WINDOWER_HOOKLIB_HOOK_HPP

#include "library.hpp"
#include "trampoline.hpp"

#include <gsl/gsl>

#include <string>
#include <utility>
#include "utilities/debug_helpers.hpp"

namespace windower::hooklib
{

template<typename, typename = void>
class hook;

template<typename R, typename... A>
class hook<R(A...), void>
{
public:
    using pointer_type = R (*)(A...);

    constexpr hook() noexcept = default;

    hook(pointer_type target, pointer_type callback) :
        m_trampoline{
            std::bit_cast<void (*)()>(target),
            std::bit_cast<void (*)()>(callback)}
    {}

    hook(library&& library, pointer_type target, pointer_type callback) :
        m_trampoline{
            std::bit_cast<void (*)()>(target),
            std::bit_cast<void (*)()>(callback)},
        m_library{std::move(library)}
    {}

    R operator()(A... args) const noexcept
    {
        return std::bit_cast<pointer_type>(m_trampoline.target())(args...);
    }

    explicit operator bool() const noexcept { return bool(m_trampoline); }

private:
    trampoline m_trampoline = {};
    library m_library;
};

template<typename R, typename... A>
class hook<R __stdcall(A...), void>
{
public:
    using pointer_type = R(__stdcall*)(A...);

    constexpr hook() noexcept = default;

    hook(pointer_type target, pointer_type callback) :
        m_trampoline{
            std::bit_cast<void (*)()>(target),
            std::bit_cast<void (*)()>(callback)}
    {}

    hook(library&& library, pointer_type target, pointer_type callback) :
        m_trampoline{
            std::bit_cast<void (*)()>(target),
            std::bit_cast<void (*)()>(callback)},
        m_library{std::move(library)}
    {}

    R operator()(A... args) const noexcept
    {
        return std::bit_cast<pointer_type>(m_trampoline.target())(args...);
    }

    explicit operator bool() const noexcept { return bool(m_trampoline); }

private:
    trampoline m_trampoline = {};
    library m_library;
};

template<typename R, typename... A>
class hook<R __fastcall(A...), void>
{
public:
    using pointer_type = R(__fastcall*)(A...);

    constexpr hook() noexcept = default;

    hook(pointer_type target, pointer_type callback) :
        m_trampoline{
            std::bit_cast<void (*)()>(target),
            std::bit_cast<void (*)()>(callback)}
    {}

    hook(library&& library, pointer_type target, pointer_type callback) :
        m_trampoline{
            std::bit_cast<void (*)()>(target),
            std::bit_cast<void (*)()>(callback)},
        m_library{std::move(library)}
    {}

    R operator()(A... args) const
    {
        return std::bit_cast<pointer_type>(m_trampoline.target())(args...);
    }

    explicit operator bool() const { return bool(m_trampoline); }

private:
    trampoline m_trampoline = {};
    library m_library;
};

template<typename R, typename... A>
class hook<R __vectorcall(A...), void>
{
public:
    using pointer_type = R(__vectorcall*)(A...);

    constexpr hook() noexcept = default;

    hook(pointer_type target, pointer_type callback) :
        m_trampoline{
            std::bit_cast<void (*)()>(target),
            std::bit_cast<void (*)()>(callback)}
    {}

    hook(library&& library, pointer_type target, pointer_type callback) :
        m_trampoline{
            std::bit_cast<void (*)()>(target),
            std::bit_cast<void (*)()>(callback)},
        m_library{std::move(library)}
    {}

    R operator()(A... args) const
    {
        return std::bit_cast<pointer_type>(m_trampoline.target())(args...);
    }

    explicit operator bool() const { return bool(m_trampoline); }

private:
    trampoline m_trampoline = {};
    library m_library;
};

template<typename R, typename T, typename... A>
class hook<R(A...), T>
{
public:
    using pointer_type = R (*)(T*, A...);

    constexpr hook() noexcept = default;

    hook(R(__fastcall* target)(T*, int, A...), pointer_type callback) :
        m_trampoline{
            std::bit_cast<void (*)()>(target),
            std::bit_cast<void (*)()>(callback),
            std::bit_cast<void (*)()>(&thunk)}
    {}

    R operator()(T* object, A... args) const noexcept
    {
        return std::bit_cast<R(__fastcall*)(T*, int, A...)>(
            m_trampoline.target())(object, 0, args...);
    }

    explicit operator bool() const noexcept { return bool(m_trampoline); }

private:
    trampoline m_trampoline = {};

    GSL_SUPPRESS("f.23")
    static R
        __fastcall thunk(T* object, pointer_type callback, A... args) noexcept
    {
        GSL_SUPPRESS("f.23") { return callback(object, args...); }
    }
};

template<typename R, typename... A>
hook<R(A...)> make_hook(R (*target)(A...), R (*callback)(A...))
{
    return {target, callback};
}

template<typename R, typename... A>
hook<R __stdcall(A...)>
make_hook(R(__stdcall* target)(A...), R(__stdcall* callback)(A...))
{
    return {target, callback};
}

template<typename R, typename... A>
hook<R __fastcall(A...)>
make_hook(R(__fastcall* target)(A...), R(__fastcall* callback)(A...))
{
    return {target, callback};
}

template<typename R, typename... A>
hook<R __vectorcall(A...)>
make_hook(R(__vectorcall* target)(A...), R(__vectorcall* callback)(A...))
{
    return {target, callback};
}

template<typename R, typename... A>
hook<R(A...)> make_hook(void (*target)(), R (*callback)(A...))
{
    return {std::bit_cast<R (*)(A...)>(target), callback};
}

template<typename R, typename... A>
hook<R __stdcall(A...)>
make_hook(void (*target)(), R(__stdcall* callback)(A...))
{
    return {std::bit_cast<R(__stdcall*)(A...)>(target), callback};
}

template<typename R, typename... A>
hook<R __fastcall(A...)>
make_hook(void (*target)(), R(__fastcall* callback)(A...))
{
    return {std::bit_cast<R(__fastcall*)(A...)>(target), callback};
}

template<typename R, typename... A>
hook<R __vectorcall(A...)>
make_hook(void (*target)(), R(__vectorcall* callback)(A...))
{
    return {std::bit_cast<R(__vectorcall*)(A...)>(target), callback};
}

template<bool Required = true, typename R, typename... A>
hook<R(A...)> make_hook(
    std::u8string_view library_name, u8zstring_view funcion_name,
    R (*callback)(A...))
{
    if (library library{library_name})
    {
        if (auto target = library.get_function(funcion_name))
        {
            return {
                std::move(library), std::bit_cast<R (*)(A...)>(target),
                callback};
        }
    }

    if constexpr (Required)
    {
        throw_system_error();
    }
    else
    {
        return {};
    }
}

template<bool Required = true, typename R, typename... A>
hook<R __stdcall(A...)> make_hook(
    std::u8string_view library_name, u8zstring_view funcion_name,
    R(__stdcall* callback)(A...))
{
    if (library library{library_name})
    {
        if (auto target = library.get_function(funcion_name))
        {
            return {
                std::move(library), std::bit_cast<R(__stdcall*)(A...)>(target),
                callback};
        }
    }

    if constexpr (Required)
    {
        throw_system_error();
    }
    else
    {
        return {};
    }
}

template<bool Required = true, typename R, typename... A>
hook<R __fastcall(A...)> make_hook(
    std::u8string_view library_name, u8zstring_view funcion_name,
    R(__fastcall* callback)(A...))
{
    if (library library{library_name})
    {
        if (auto target = library.get_function(funcion_name))
        {
            return {
                std::move(library), std::bit_cast<R(__fastcall*)(A...)>(target),
                callback};
        }
    }

    if constexpr (Required)
    {
        throw_system_error();
    }
    else
    {
        return {};
    }
}

template<bool Required = true, typename R, typename... A>
hook<R __vectorcall(A...)> make_hook(
    std::u8string_view library_name, u8zstring_view funcion_name,
    R(__vectorcall* callback)(A...))
{
    if (library library{library_name})
    {
        if (auto target = library.get_function(funcion_name))
        {
            return {
                std::move(library),
                std::bit_cast<R(__vectorcall*)(A...)>(target), callback};
        }
    }

    if constexpr (Required)
    {
        throw_system_error();
    }
    else
    {
        return {};
    }
}

template<typename R, typename T, typename... A>
hook<R(A...), T> make_hook_thiscall(
    R(__fastcall* target)(T*, int, A...), R (*callback)(T*, A...))
{
    return {target, callback};
}

template<typename R, typename T, typename... A>
hook<R(A...), T> make_hook_thiscall(void (*target)(), R (*callback)(T*, A...))
{
    return {std::bit_cast<R(__fastcall*)(T*, int, A...)>(target), callback};
}

}

#endif
