#ifndef WINDOWER_UTILITIES_DEBUG_HELPERS_HPP
#define WINDOWER_UTILITIES_DEBUG_HELPERS_HPP

#include "utilities/string_helpers.hpp"

#include <cstdint>
#include <iosfwd>
#include <span>
#include <system_error>

namespace windower
{
    void set_thread_name(u8zstring_view name);

    std::ostream& hex_dump(std::ostream& stream, std::span<std::byte const> buffer);

    [[noreturn]] void throw_system_error(std::uint32_t error);
    [[noreturn]] void throw_system_error();
    [[noreturn]] void fail_fast() noexcept;
}

#endif
