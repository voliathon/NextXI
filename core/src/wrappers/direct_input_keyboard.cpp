#include "wrappers/direct_input_keyboard.hpp"
#include "wrappers/direct_input_device.hpp"
#include "addon/modules/event.hpp"
#include "addon/unsafe.hpp"
#include "core.hpp"
#include "ui/engine_console.hpp"
#include "ui/user_interface.hpp"

#include <dinput.h>
#include <lua.hpp>
#include <algorithm>
#include <array>
#include <cstdio>
#include <vector>

static std::array<::BYTE, 256> s_last_key_state = { 0 };

static void dispatch_key_to_lua(std::uint8_t dik, bool down)
{
    windower::run_on_all_interpreters([dik, down](windower::lua::state s) {
        auto L = windower::lua::unsafe::unwrap(s);

        // Fetch package.loaded['core.windower']
        lua_getglobal(L, "package");
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            return;
        }

        lua_getfield(L, -1, "loaded");
        if (!lua_istable(L, -1)) {
            lua_pop(L, 2);
            return;
        }

        lua_getfield(L, -1, "core.windower");
        if (!lua_istable(L, -1)) {
            lua_pop(L, 3);
            return;
        }

        lua_getfield(L, -1, "trigger_event");
        if (lua_isfunction(L, -1)) {
            lua_pushstring(L, "keyboard");
            lua_pushinteger(L, dik);
            lua_pushboolean(L, down ? 1 : 0);
            lua_pushinteger(L, 0);      // flags
            lua_pushboolean(L, 0);      // blocked

            if (lua_pcall(L, 5, 2, 0) == 0) {
                lua_pop(L, 2);          // Pop return values
            }
            else {
                lua_pop(L, 1);          // Pop error message
            }
        }
        else {
            lua_pop(L, 1);
        }

        lua_pop(L, 3); // Pop core.windower, loaded, package
        });
}

windower::direct_input_keyboard::direct_input_keyboard(
    ::IDirectInputDevice8A* impl) noexcept :
    direct_input_device{ impl }
{
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_keyboard::GetDeviceState(
    ::DWORD cbData, void* lpvData) noexcept
{
    if (!lpvData)
    {
        return E_POINTER;
    }

    auto keys = static_cast<::BYTE*>(lpvData);
    auto& core = core::instance();

    if (core.ui.m_console && core.ui.m_console->is_visible())
    {
        std::memset(keys, 0, cbData);
    }
    else
    {
        auto& state = core.binding_manager.client_state();
        std::copy_n(
            state.begin(), std::min(state.size(), std::size_t(cbData)), keys);
    }

    // Check for key transitions and dispatch to Lua
    const std::size_t scan_size = std::min<std::size_t>(cbData, 256);
    for (std::size_t i = 0; i < scan_size; ++i)
    {
        bool was_down = (s_last_key_state[i] & 0x80) != 0;
        bool is_down = (keys[i] & 0x80) != 0;

        if (is_down && !was_down)
        {
            dispatch_key_to_lua(static_cast<std::uint8_t>(i), true);
        }
        else if (!is_down && was_down)
        {
            dispatch_key_to_lua(static_cast<std::uint8_t>(i), false);
        }
    }

    std::copy_n(keys, scan_size, s_last_key_state.begin());

    return DI_OK;
}

::HRESULT STDMETHODCALLTYPE windower::direct_input_keyboard::GetDeviceData(
    ::DWORD, ::DIDEVICEOBJECTDATA* rgdod, ::DWORD* pdwInOut, ::DWORD) noexcept
{
    if (!rgdod || !pdwInOut)
    {
        return E_POINTER;
    }

    *pdwInOut = INFINITE;
    direct_input_device::GetDeviceData(
        sizeof(::DIDEVICEOBJECTDATA), nullptr, pdwInOut, 0);
    *pdwInOut = 0;
    return DI_OK;
}
