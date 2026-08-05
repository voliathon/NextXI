#include "handle.hpp"

#include "utility.hpp"
#include "utilities/debug_helpers.hpp"

#include <windows.h>

#include <system_error>
#include <utility>

windower::handle::handle() noexcept : m_handle{INVALID_HANDLE_VALUE} {}

windower::handle::handle(handle const& other) : m_handle{INVALID_HANDLE_VALUE}
{
    if (other)
    {
        auto process = ::GetCurrentProcess();

        auto handle = ::HANDLE{};
        if (!::DuplicateHandle(
                process, other.m_handle, process, &handle, 0, false,
                DUPLICATE_SAME_ACCESS))
        {
            throw std::system_error{
                std::error_code(::GetLastError(), std::system_category())};
        }

        m_handle = handle;
    }
}

windower::handle::handle(handle&& other) noexcept : m_handle{other.m_handle}
{
    other.m_handle = INVALID_HANDLE_VALUE;
}

windower::handle::handle(void* handle) noexcept : m_handle{handle} {}

windower::handle::~handle()
{
    if (*this && !::CloseHandle(m_handle))
    {
        fail_fast();
    }
}

windower::handle& windower::handle::operator=(handle const& other) noexcept
{
    using std::swap;

    if (this != &other)
    {
        auto temp = other;
        swap(*this, temp);
    }

    return *this;
}

windower::handle& windower::handle::operator=(handle&& other) noexcept
{
    handle::~handle();
    m_handle       = nullptr;
    m_handle       = other.m_handle;
    other.m_handle = nullptr;
    return *this;
}

windower::handle::operator bool() const noexcept
{
    return m_handle && m_handle != INVALID_HANDLE_VALUE;
}

windower::handle::operator void*() const noexcept { return m_handle; }
