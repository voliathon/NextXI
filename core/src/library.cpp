#include "library.hpp"

#include "utility.hpp"
#include "utilities/debug_helpers.hpp"

#include <windows.h>

#include <gsl/gsl>

#include <string>
#include <system_error>
#include <utility>

windower::library::library(library const& other)
{
    if (other)
    {
        ::HMODULE temp = nullptr;
        if (!::GetModuleHandleExW(
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                static_cast<::LPCWSTR>(other.m_handle), &temp))
        {
            throw std::system_error{
                gsl::narrow_cast<int>(::GetLastError()),
                std::system_category()};
        }
        m_handle = temp;
    }
}

windower::library::library(library&& other) noexcept :
    m_handle{std::move(other.m_handle)}
{
    other.m_handle = nullptr;
}

windower::library::library(std::filesystem::path const& path)
{
    auto temp = ::LoadLibraryW(path.c_str());
    if (!temp)
    {
        auto const error = ::GetLastError();
        if (error != ERROR_MOD_NOT_FOUND)
        {
            throw std::system_error{
                gsl::narrow_cast<int>(error), std::system_category()};
        }
    }
    m_handle = temp;
}

windower::library::~library()
{
    if (*this && !::FreeLibrary(static_cast<::HMODULE>(m_handle)))
    {
        fail_fast();
    }
}

windower::library& windower::library::operator=(library const& other)
{
    using std::swap;

    if (this != &other)
    {
        auto copy = other;
        swap(*this, copy);
    }

    return *this;
}

windower::library& windower::library::operator=(library&& other) noexcept
{
    library::~library();
    m_handle       = nullptr;
    m_handle       = other.m_handle;
    other.m_handle = nullptr;
    return *this;
}

windower::library::operator bool() const noexcept
{
    return m_handle && m_handle != INVALID_HANDLE_VALUE;
}

void (*windower::library::get_function(u8zstring_view name) const noexcept)()
{
    auto const view = to_zstring_view(name);
    auto ptr        = ::GetProcAddress(*this, view.c_str());
    WINDOWER_SUPPRESS("type.1") { return reinterpret_cast<void (*)()>(ptr); }
}
