#include "ui/cursor.hpp"

#include "utility.hpp"
#include "utilities/debug_helpers.hpp"

#include <windows.h>

#include <utility>

namespace windower::ui
{

cursor::cursor(cursor const& other) noexcept :
    m_handle{::CopyImage(
        static_cast<::HANDLE>(other.m_handle), IMAGE_CURSOR, 0, 0, 0)}
{}

cursor::cursor(cursor&& other) noexcept : m_handle{std::move(other.m_handle)}
{
    other.m_handle = nullptr;
}

cursor::cursor(std::filesystem::path const& path) noexcept :
    m_handle{::LoadCursorFromFileW(path.c_str())}
{}

cursor::~cursor() noexcept
{
    if (*this && !::DestroyCursor(static_cast<::HCURSOR>(m_handle)) &&
        ::GetLastError() != ERROR_SUCCESS)
    {
        fail_fast();
    }
    m_handle = nullptr;
}

cursor& cursor::operator=(cursor const& other) noexcept
{
    using std::swap;

    if (this != &other)
    {
        auto temp = other;
        swap(*this, temp);
    }

    return *this;
}

cursor& cursor::operator=(cursor&& other) noexcept
{
    cursor::~cursor();
    m_handle       = nullptr;
    m_handle       = other.m_handle;
    other.m_handle = nullptr;
    return *this;
}

cursor::operator bool() const noexcept
{
    return m_handle && m_handle != INVALID_HANDLE_VALUE;
}

}
