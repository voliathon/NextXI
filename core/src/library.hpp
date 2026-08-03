#ifndef WINDOWER_LIBRARY_HPP
#define WINDOWER_LIBRARY_HPP

#include "utility.hpp"

#include <gsl/gsl>

#include <filesystem>
#include <string_view>
#include <type_traits>

namespace windower
{

class library
{
public:
    library() noexcept = default;
    library(library const&);
    library(library&&) noexcept;
    library(std::filesystem::path const&);

    ~library();

    library& operator=(library const&);
    library& operator=(library&&) noexcept;
    explicit operator bool() const noexcept;

    template<typename T>
    operator T*() const noexcept
    {
        return static_cast<T*>(m_handle);
    }

    void (*get_function(u8zstring_view) const noexcept)();

    template<typename F>
    requires std::is_function_v<std::remove_pointer_t<F>>
        std::add_pointer_t<std::remove_pointer_t<F>>
        get_function(u8zstring_view name)
    const noexcept
    {
        GSL_SUPPRESS("type.1")
        {
            return reinterpret_cast<std::remove_pointer_t<F>*>(
                get_function(name));
        }
    }

private:
    void* m_handle = nullptr;
};

}

#endif