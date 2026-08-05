#include "ui/command_buffer.hpp"

#include <d3d8.h>

#include <gsl/gsl>

#include <algorithm>
#include <bit>
#include <cstdint>

namespace windower::ui
{

void command_buffer::wrapped_command::execute(
    ::IDirect3DDevice8* d3d_device) const noexcept
{
    m_execute(m_data, d3d_device);
}

std::uintptr_t command_buffer::state(state_id id) const noexcept
{
    return gsl::at(m_state, gsl::narrow_cast<std::size_t>(id));
}

bool command_buffer::has_state(state_id id, std::uintptr_t value) const noexcept
{
    return state(id) == value;
}

bool command_buffer::has_state(state_id id, void const* value) const noexcept
{
    return has_state(id, std::bit_cast<std::uintptr_t>(value));
}

void command_buffer::state(state_id id, std::uintptr_t value) noexcept
{
    gsl::at(m_state, gsl::narrow_cast<std::size_t>(id)) = value;
}

void command_buffer::state(state_id id, void const* value) noexcept
{
    state(id, std::bit_cast<std::uintptr_t>(value));
}

void command_buffer::execute(::IDirect3DDevice8* device) const noexcept
{
    for (auto const& c : m_commands)
    {
        c.execute(device);
    }
}

void command_buffer::clear() noexcept
{
    namespace range = std::ranges;

    m_commands.clear();
    range::fill(m_state, 0);
}

}