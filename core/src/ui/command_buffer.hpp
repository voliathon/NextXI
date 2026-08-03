#ifndef WINDOWER_UI_COMMAND_BUFFER_HPP
#define WINDOWER_UI_COMMAND_BUFFER_HPP

#include "ui/static_any.hpp"

#include <d3d8.h>

#include <array>
#include <concepts>
#include <cstddef>
#include <utility>
#include <vector>

namespace windower::ui
{

constexpr std::size_t max_command_size = 16;

class command_buffer;

template<typename T>
concept command = requires(T const& value, ::IDirect3DDevice8* d3d_device)
{
    requires sizeof(T) <= max_command_size;
    requires alignof(T) <= alignof(std::max_align_t);
    { value.execute(d3d_device) } noexcept;
};

template<typename T, typename... A>
concept stitchable_command = requires(T const& value, A&&... args)
{
    requires command<T>;
    { value.stitch(std::forward<A>(args)...) } noexcept;
};

template<typename T, typename... A>
concept stateful_command =
    requires(T const& value, command_buffer& command_bufer, A&&... args)
{
    requires command<T>;
    { T::check_state(command_bufer, std::forward<A>(args)...) } noexcept ->
        std::convertible_to<bool>;
};

class command_buffer
{
public:
    enum class state_id
    {
        vertex_buffer,
        index_buffer,
        texture,
    };

    template<command C, typename... A>
    void emplace(A&&... args) noexcept
    {
        auto const enqueue = [&, this]() noexcept {
            if constexpr (stateful_command<C, A...>)
            {
                return C::check_state(*this, std::forward<A>(args)...);
            }
            else
            {
                return true;
            }
        }();

        if (enqueue)
        {
            if constexpr (stitchable_command<C, A...>)
            {
                if (!m_commands.empty())
                {
                    if (auto ptr = m_commands.back().as<C>())
                    {
                        ptr->stitch(std::forward<A>(args)...);
                        return;
                    }
                }
            }
            m_commands.emplace_back(
                std::in_place_type<C>, std::forward<A>(args)...);
        }
    }

    std::uintptr_t state(state_id id) const noexcept;
    bool has_state(state_id id, std::uintptr_t value) const noexcept;
    bool has_state(state_id id, void const* value) const noexcept;
    void state(state_id id, std::uintptr_t value) noexcept;
    void state(state_id id, void const* value) noexcept;

    void execute(::IDirect3DDevice8* device) const noexcept;
    void clear() noexcept;

private:
    class wrapped_command
    {
    public:
        template<command C, typename... A>
        wrapped_command(std::in_place_type_t<C> tag, A&&... args) noexcept :
            m_execute{[](static_any<max_command_size> const& data,
                         ::IDirect3DDevice8* d3d_device) noexcept {
                data.value<C>()->execute(d3d_device);
            }},
            m_data{tag, std::forward<A>(args)...}
        {}

        template<command C>
        auto as() noexcept
        {
            return m_data.value<C>();
        }

        void execute(::IDirect3DDevice8* d3d_device) const noexcept;

    private:
        void (*m_execute)(
            static_any<max_command_size> const& data,
            ::IDirect3DDevice8* d3d_device) noexcept;
        static_any<max_command_size> m_data;
    };

    std::vector<wrapped_command> m_commands;
    std::array<std::uintptr_t, 3> m_state = {};
};

}

#endif