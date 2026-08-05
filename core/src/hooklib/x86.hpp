#ifndef WINDOWER_HOOKLIB_X86_HPP
#define WINDOWER_HOOKLIB_X86_HPP

#include <gsl/gsl>

#include <array>
#include <cstddef>
#include <cstdint>

namespace windower::hooklib
{

class x86
{
public:
    class jump
    {
    public:
        jump(void*);

    private:
        std::array<std::uint8_t, 5> m_raw;
    };

    class thiscall_thunk
    {
    public:
        thiscall_thunk(void*, void*);

    private:
        std::array<std::uint8_t, 5> m_raw;
        jump m_jump;
    };

    static constexpr std::size_t max_instruction_size         = 15;
    static constexpr std::uint8_t nop_instruction             = 0x90;
    static constexpr std::uint8_t trap_instruction            = 0xCC;
    static unsigned short int const hotpatch_jump_instruction = 0xF9EB;

    static gsl::not_null<std::uint8_t*>
        follow_jumps(gsl::not_null<std::uint8_t*>) noexcept;
    static gsl::not_null<std::uint8_t*>
        next_instruction(gsl::not_null<std::uint8_t*>);
    static bool is_hotpatchable(gsl::not_null<std::uint8_t*>);
};

}

#endif