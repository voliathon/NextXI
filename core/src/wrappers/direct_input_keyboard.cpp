#include "wrappers/direct_input_keyboard.hpp"

#include "wrappers/direct_input_device.hpp"
#include "core.hpp"

#include <dinput.h>

#include <vector>

windower::direct_input_keyboard::direct_input_keyboard(
    ::IDirectInputDevice8A* impl) noexcept :
    direct_input_device{impl}
{}

::HRESULT STDMETHODCALLTYPE windower::direct_input_keyboard::GetDeviceState(
    ::DWORD cbData, void* lpvData) noexcept
{
    if (!lpvData)
    {
        return E_POINTER;
    }

    auto keys = static_cast<::BYTE*>(lpvData);
    auto& state = core::instance().binding_manager.client_state();
    std::copy_n(
        state.begin(), std::min(state.size(), std::size_t(cbData)), keys);

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