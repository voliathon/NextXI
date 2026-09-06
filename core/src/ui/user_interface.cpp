#include "ui/user_interface.hpp"

#include "ui/context.hpp"
#include "ui/dimension.hpp"
#include "ui/engine_console.hpp"
#include "utilities/paths.hpp"

#include <d3d8.h>
#include <imgui.h>

#include <gsl/gsl>
#include <memory>
#include <filesystem>
#include <string>

namespace windower
{

    user_interface::operator bool() const noexcept { return m_context != nullptr; }

    void user_interface::initialize(
        ::HWND hwnd, gsl::not_null<::IDirect3DDevice8*> d3d_device,
        ui::dimension const& screen_size, ui::dimension const& ui_size,
        ui::dimension const& render_size) noexcept
    {
        // 1. CREATE ENGINE CONTEXTS FIRST
        // m_console creates the ImGui context, so it MUST exist before we touch fonts!
        m_context = std::make_unique<ui::context>(
            hwnd, d3d_device, screen_size, ui_size, render_size);
        m_console = std::make_unique<ui::engine_console>();

        // 2. INJECT CUSTOM FONT ATLAS
        if (ImGui::GetCurrentContext()) {
            ImGuiIO& io = ImGui::GetIO();

            // Clear whatever engine_console might have put in here by default
            io.Fonts->Clear();

            // Always load the default ImGui font first (Index 0)
            io.Fonts->AddFontDefault();

            // Helper lambda to silently attempt loading a font if the file exists
            auto try_load_font = [&](std::filesystem::path const& path) {
                std::error_code ec;
                if (std::filesystem::exists(path, ec)) {
                    ImFontConfig font_cfg;
                    font_cfg.OversampleH = 2;
                    font_cfg.OversampleV = 2;
                    io.Fonts->AddFontFromFileTTF(path.string().c_str(), 16.0f, &font_cfg);
                }
                };

            // Scan NextXI/assets/fonts/ for user-provided .ttf files
            std::error_code ec;
            auto custom_font_dir = windower::user_path() / u8"assets" / u8"fonts";
            if (std::filesystem::exists(custom_font_dir, ec)) {
                for (auto const& entry : std::filesystem::directory_iterator(custom_font_dir, ec)) {
                    if (entry.path().extension() == ".ttf" || entry.path().extension() == ".TTF") {
                        try_load_font(entry.path());
                    }
                }
            }

            // Fallback to common Windows system fonts
            try_load_font("C:\\Windows\\Fonts\\arial.ttf");
            try_load_font("C:\\Windows\\Fonts\\consola.ttf");
            try_load_font("C:\\Windows\\Fonts\\trebuc.ttf");
            try_load_font("C:\\Windows\\Fonts\\tahoma.ttf");
            try_load_font("C:\\Windows\\Fonts\\verdana.ttf");
            try_load_font("C:\\Windows\\Fonts\\comic.ttf");
            try_load_font("C:\\Windows\\Fonts\\times.ttf");

            // Build the atlas immediately so the NextXI backend handles the rest
            io.Fonts->Build();
        }
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

    void user_interface::render(ui::layer layer) noexcept {
        if (m_context)
        {
            if (layer == ui::layer::screen && m_console)
            {
                m_console->render(*m_context);
            }
        }
    }

    ui::context* user_interface::context() noexcept { return m_context.get(); }

}
