#include "ui/user_interface.hpp"

#include "ui/context.hpp"
#include "ui/dimension.hpp"
#include "ui/engine_console.hpp"

#include <d3d8.h>

#include <gsl/gsl>

#include <memory>

namespace windower
{

user_interface::operator bool() const noexcept { return m_context != nullptr; }

void user_interface::initialize(
    ::HWND hwnd, gsl::not_null<::IDirect3DDevice8*> d3d_device,
    ui::dimension const& screen_size, ui::dimension const& ui_size,
    ui::dimension const& render_size) noexcept
{
    m_context = std::make_unique<ui::context>(
        hwnd, d3d_device, screen_size, ui_size, render_size);
    m_console = std::make_unique<ui::engine_console>();
}

void user_interface::reset() noexcept
{
    m_console.reset();
    m_context.reset();
}

void user_interface::activate_next_window() noexcept
{
    if (m_context)
    {
        m_context->activate_next_window();
    }
}

void user_interface::activate_previous_window() noexcept
{
    if (m_context)
    {
        m_context->activate_previous_window();
    }
}

std::optional<::LRESULT>
user_interface::process_message(::MSG const& message) const noexcept
{
    if (m_console)
    {
        if (auto result = m_console->process_message(message))
        {
            return result;
        }
    }
    return m_context ? m_context->process_message(message) : std::nullopt;
}

void user_interface::begin_frame() noexcept
{
    if (m_context)
    {
        m_context->begin_frame();
    }
}

void user_interface::end_frame() noexcept
{
    if (m_context)
    {
        m_context->end_frame();
    }
}

void user_interface::render(ui::layer layer) noexcept
{
    if (m_context)
    {
        if (layer == ui::layer::screen && m_console)
        {
            m_console->render(*m_context);
        }
        m_context->render(layer);
    }
}

ui::context* user_interface::context() noexcept { return m_context.get(); }

}