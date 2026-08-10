#include "wrappers/direct_3d_device.hpp"
#include "addon/addon_manager.hpp"
#include "addon/script_environment.hpp"
#include "addon/modules/event.hpp"
#include "command_manager.hpp"
#include "core.hpp"
#include "hooks/ffximain.hpp"
#include "wrappers/direct_3d.hpp"

#include <windows.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx8.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3d8.h>
#include <cstddef>
#include <memory>
#include <string>
#include <stdio.h> // Required for our macro!
#include <array>   // Added for std::array to satisfy bounds checking!

// --- NEXTXI LOGGING MACRO ---
#ifdef _DEBUG
#define NEXTXI_LOG(format, ...) \
    { \
        std::array<char, 512> buffer{}; \
        sprintf_s(buffer.data(), buffer.size(), "[NextXI-Telemetry] " format "\n", __VA_ARGS__); \
        ::OutputDebugStringA(buffer.data()); \
    }
#else
#define NEXTXI_LOG(format, ...) do {} while(0)
#endif
// ----------------------------

static bool g_imgui_initialized = false;

// Use Invalidate/Create instead of Shutdown to survive Alt-Tab and dragging!
extern IMGUI_IMPL_API void ImGui_ImplDX8_Shutdown();
extern IMGUI_IMPL_API void ImGui_ImplWin32_Shutdown();

windower::direct_3d_device::direct_3d_device(
    ::IDirect3DDevice8* impl, ::HWND hwnd, direct_3d* parent) :
    m_impl{ impl },
    m_parent{ parent }
{
    m_parent->AddRef();
    AddRef();

    auto& core = core::instance();
    core.incoming_packet_queue =
        std::make_unique<packet_queue>(packet_direction::incoming);
    core.outgoing_packet_queue =
        std::make_unique<packet_queue>(packet_direction::outgoing);

    ffximain::install();

    core.run_on_next_frame([impl, hwnd] {
        auto& core = core::instance();

        core.ui.initialize(
            hwnd, impl,
            { gsl::narrow_cast<float>(core.settings.window_bounds.size.width),
             gsl::narrow_cast<float>(core.settings.window_bounds.size.height) },
            { gsl::narrow_cast<float>(core.settings.ui_size.width),
             gsl::narrow_cast<float>(core.settings.ui_size.height) },
            { gsl::narrow_cast<float>(core.settings.render_size.width),
             gsl::narrow_cast<float>(core.settings.render_size.height) });
        core.script_environment.reset();
        core.addon_manager = std::make_unique<addon_manager>();
        try
        {
            core.script_environment.execute(u8"init");
        }
        catch (std::exception const& e)
        {
            ::windower::core::error(u8"core", e, ::windower::command_source::console);
        }
        });
}

windower::direct_3d_device::~direct_3d_device()
{
    auto& core = core::instance();
    core.script_environment.reset();
    core.addon_manager = nullptr;
    core.ui.reset();

    ffximain::uninstall();

    core.incoming_packet_queue = nullptr;
    core.outgoing_packet_queue = nullptr;

    m_impl->Release();
    m_impl = nullptr;

    m_parent->Release();
    m_parent = nullptr;
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::QueryInterface(
    ::IID const& riid, void** ppvObj) noexcept
{
    if (!ppvObj)
    {
        return E_POINTER;
    }
    *ppvObj = nullptr;

    if (::IsEqualGUID(riid, ::IID_IUnknown))
    {
        AddRef();
        ::IUnknown* const ptr = this;
        *ppvObj = ptr;
        return S_OK;
    }
    else if (::IsEqualGUID(riid, ::IID_IDirect3DDevice8))
    {
        AddRef();
        ::IDirect3DDevice8* const ptr = this;
        *ppvObj = ptr;
        return S_OK;
    }
    return E_NOINTERFACE;
}

::ULONG STDMETHODCALLTYPE windower::direct_3d_device::AddRef() noexcept
{
    return ::InterlockedIncrement(&m_count);
}

::ULONG STDMETHODCALLTYPE windower::direct_3d_device::Release() noexcept
{
    auto const count = ::InterlockedDecrement(&m_count);
    if (count == 0)
    {
        delete this;
    }
    return count;
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::Reset(
    ::D3DPRESENT_PARAMETERS* pPresentationParameters) noexcept
{
    // Explicitly cast the UINT struct fields to (int) to match the %d format!
    NEXTXI_LOG("Reset() Triggered! Windowed: %d | Width: %d | Height: %d | RefreshRate: %d",
        pPresentationParameters ? (int)pPresentationParameters->Windowed : -1,
        pPresentationParameters ? (int)pPresentationParameters->BackBufferWidth : -1,
        pPresentationParameters ? (int)pPresentationParameters->BackBufferHeight : -1,
        pPresentationParameters ? (int)pPresentationParameters->FullScreen_RefreshRateInHz : -1);

    // Marked as const per Core Guidelines
    const HRESULT hr = m_impl->Reset(pPresentationParameters);

    NEXTXI_LOG("Reset() Result HRESULT: 0x%08X", hr);

    if (SUCCEEDED(hr) && g_imgui_initialized)
    {
        NEXTXI_LOG("Reset Successful. Rebuilding ImGui DX8 Backend...");
        ImGui_ImplDX8_Shutdown();
        ImGui_ImplDX8_Init(m_impl);
        NEXTXI_LOG("ImGui Rebuild Complete.");
    }
    else if (!SUCCEEDED(hr))
    {
        NEXTXI_LOG("CRITICAL WARNING: Reset() FAILED!");
    }

    return hr;
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::Present(
    ::RECT const* pSourceRect, ::RECT const* pDestRect,
    ::HWND hDestWindowOverride, RGNDATA const* pDirtyRegion) noexcept
{
    auto& core = core::instance();

    // --- ULTIMATE RADAR: TRACK THE THIEF AT 60 FPS ---
    HWND target_hwnd = hDestWindowOverride ? hDestWindowOverride : static_cast<HWND>(core.client_hwnd);
    if (target_hwnd)
    {
        static LONG last_style = 0;
        // Marked as const
        const LONG current_style = ::GetWindowLongW(target_hwnd, GWL_STYLE);

        if (last_style != 0 && current_style != last_style)
        {
            NEXTXI_LOG("THIEF CAUGHT: Style mutated from 0x%08X to 0x%08X outside of hooks!", last_style, current_style);
        }
        last_style = current_style;

        static int last_w = 0, last_h = 0;
        RECT r;
        if (::GetWindowRect(target_hwnd, &r))
        {
            // Marked as const
            const int w = r.right - r.left;
            const int h = r.bottom - r.top;
            if (last_w != 0 && (w != last_w || h != last_h))
            {
                NEXTXI_LOG("THIEF CAUGHT: Window Rect mutated from %dx%d to %dx%d!", last_w, last_h, w, h);
            }
            last_w = w; last_h = h;
        }
    }
    // --- RADAR END ---

    if (!g_imgui_initialized)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigWindowsResizeFromEdges = true;
        ImGui::StyleColorsDark();

        ImGui_ImplWin32_Init(target_hwnd);
        ImGui_ImplDX8_Init(m_impl);
        g_imgui_initialized = true;
    }

    if (g_imgui_initialized)
    {
        ImGui_ImplDX8_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        windower::run_on_all_interpreters([](windower::lua::state s) {
            windower::lua::stack_guard guard{ s };
            windower::lua::push(guard, u8"imgui_render");
            windower::lua::raw_get(guard, windower::lua::globals);

            if (windower::lua::typeof(guard, -1) == windower::lua::type::function)
            {
                windower::lua::call(guard, 0, 0);
            }
            });
    }

    m_impl->BeginScene();

    core.update();
    core.end_frame();

    core.ui.render(windower::ui::layer::screen);
    core.ui.render(windower::ui::layer::layout);

    if (g_imgui_initialized)
    {
        ImGui::Render();

        D3DVIEWPORT8 ffxi_viewport;
        m_impl->GetViewport(&ffxi_viewport);

        IDirect3DSurface8* backbuffer = nullptr;
        if (SUCCEEDED(m_impl->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backbuffer)) && backbuffer)
        {
            D3DSURFACE_DESC desc;
            backbuffer->GetDesc(&desc);
            backbuffer->Release();

            D3DVIEWPORT8 imgui_viewport;
            imgui_viewport.X = 0;
            imgui_viewport.Y = 0;
            imgui_viewport.Width = desc.Width;
            imgui_viewport.Height = desc.Height;
            imgui_viewport.MinZ = 0.0f;
            imgui_viewport.MaxZ = 1.0f;

            m_impl->SetViewport(&imgui_viewport);
        }

        ImGui_ImplDX8_RenderDrawData(ImGui::GetDrawData());
        m_impl->SetViewport(&ffxi_viewport);
    }

    m_impl->EndScene();
    m_frame_in_progress = false;

    return m_impl->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::BeginScene() noexcept
{
    auto const result = m_impl->BeginScene();
    if (!m_frame_in_progress)
    {
        m_frame_in_progress = true;
        core::instance().begin_frame();
    }
    return result;
}

::HRESULT STDMETHODCALLTYPE windower::direct_3d_device::EndScene() noexcept
{
    return m_impl->EndScene();
}
