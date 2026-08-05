#include "debug_helpers.hpp"

#include "library.hpp"
#include "unicode.hpp"
#include "utility.hpp" 

#include <windows.h>
#include <gsl/gsl>

#include <iomanip>
#include <ostream>

#if defined(_MSC_VER)
namespace
{
    constexpr auto MS_VC_EXCEPTION = ::DWORD{ 0x406D1388 };

#pragma pack(push, 8)
    struct threadname_info
    {
        ::DWORD dwType = 0x1000;
        ::LPCSTR szName;
        ::DWORD dwThreadID = 0xFFFFFFFF;
        ::DWORD dwFlags = 0;
    };
#pragma pack(pop)

    void set_thread_name_seh(windower::zstring_view name)
    {
        __try
        {
            threadname_info const info{ .szName = name.c_str() };
            ::RaiseException(
                MS_VC_EXCEPTION, 0, sizeof info / sizeof(::ULONG_PTR),
                reinterpret_cast<::ULONG_PTR const*>(&info));
        }
        __except (
            ::GetExceptionCode() == MS_VC_EXCEPTION ? EXCEPTION_EXECUTE_HANDLER
            : EXCEPTION_CONTINUE_SEARCH)
        {
            __noop();
        }
    }
} // namespace

void windower::set_thread_name(u8zstring_view name)
{
    if (auto const kernel32 = library{ u8"kernel32.dll" })
    {
        if (auto const SetThreadDescription =
            kernel32.get_function<decltype(::SetThreadDescription)>(
                u8"SetThreadDescription"))
        {
            auto const result = SetThreadDescription(
                ::GetCurrentThread(), to_wstring(name).c_str());
            if (SUCCEEDED(result))
            {
                return;
            }
        }
    }
    set_thread_name_seh(to_zstring_view(name));
}
#endif

std::ostream& windower::hex_dump(std::ostream& stream, std::span<std::byte const> buffer)
{
    auto line = buffer.begin();
    auto const end = buffer.end();

    format_guard format{ stream };

    stream.setf(std::ios::hex, std::ios_base::basefield);
    stream.fill('0');
    for (; std::distance(line, end) >= 16; std::advance(line, 16))
    {
        stream << &*line << ':' << ' ';
        for (auto i = 0; i < 16; ++i)
        {
            stream.width(2);
            stream << std::to_integer<std::uint8_t>(*std::next(line, i)) << ' ';
        }
        stream << ' ';
        for (auto i = 0; i < 16; ++i)
        {
            auto const c = std::to_integer<char>(*std::next(line, i));
            stream << (c >= 0x20 && c <= 0x7E ? c : '.');
        }
        stream << std::endl;
    }

    auto const extra = std::distance(line, end);
    if (extra)
    {
        stream << &*line << ':' << ' ';
        for (auto i = 0; i < extra; ++i)
        {
            stream.width(2);
            stream << std::to_integer<std::uint8_t>(*std::next(line, i)) << ' ';
        }
        for (int i = extra; i < 16; ++i)
        {
            stream << ' ' << ' ' << ' ';
        }
        stream << ' ';
        for (auto i = 0; i < extra; ++i)
        {
            auto const c = std::to_integer<char>(*std::next(line, i));
            stream << (c >= 0x20 && c <= 0x7E ? c : '.');
        }
        for (int i = extra; i < 16; ++i)
        {
            stream << ' ';
        }
        stream << std::endl;
    }

    return stream;
}

[[noreturn]] void windower::throw_system_error(std::uint32_t error)
{
    throw std::system_error{
        gsl::narrow_cast<int>(error), std::system_category() };
}

[[noreturn]] void windower::throw_system_error()
{
    throw_system_error(::GetLastError());
}

[[noreturn]] void windower::fail_fast() noexcept
{
    WINDOWER_DEBUG_BREAK;
    std::_Exit(-1);
}
