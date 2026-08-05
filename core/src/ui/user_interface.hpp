#ifndef WINDOWER_UI_USER_INTERFACE_HPP
#define WINDOWER_UI_USER_INTERFACE_HPP

#include "ui/context.hpp"
#include "ui/dimension.hpp"
#include "ui/layer.hpp"
#include "ui/engine_console.hpp"

#include <windows.h>

#include <d3d8.h>

#include <gsl/gsl>

#include <memory>
#include <optional>

namespace windower
{

class user_interface
{
public:
    explicit operator bool() const noexcept;

    ui::context* context() noexcept;

    void initialize(
        ::HWND hwnd, gsl::not_null<::IDirect3DDevice8*> d3d_device,
        ui::dimension const& screen_size, ui::dimension const& ui_size,
        ui::dimension const& render_size) noexcept;
    void reset() noexcept;

    void activate_next_window() noexcept;
    void activate_previous_window() noexcept;

    std::optional<::LRESULT>
    process_message(::MSG const& message) const noexcept;
    void begin_frame() noexcept;
    void end_frame() noexcept;
    void render(ui::layer layer) noexcept;

private:
    std::unique_ptr<ui::context> m_context;
    std::unique_ptr<ui::engine_console> m_console;
};

}

#endif