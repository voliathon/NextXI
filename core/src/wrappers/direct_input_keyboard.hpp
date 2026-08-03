#ifndef WINDOWER_WRAPPERS_DIRECT_INPUT_KEYBOARD_HPP
#define WINDOWER_WRAPPERS_DIRECT_INPUT_KEYBOARD_HPP

#include "direct_input_device.hpp"

#include <dinput.h>

namespace windower
{

class direct_input_keyboard : public direct_input_device
{
public:
    direct_input_keyboard() = delete;
    direct_input_keyboard(::IDirectInputDevice8A*) noexcept;
    direct_input_keyboard(direct_input_keyboard const&) = delete;
    direct_input_keyboard(direct_input_keyboard&&) = delete;
    ~direct_input_keyboard() override = default;

    direct_input_keyboard& operator=(direct_input_keyboard const&) = delete;
    direct_input_keyboard& operator=(direct_input_keyboard&&) = delete;

    ::HRESULT STDMETHODCALLTYPE
    GetDeviceState(::DWORD, void*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE GetDeviceData(
        ::DWORD, ::DIDEVICEOBJECTDATA*, ::DWORD*, ::DWORD) noexcept override;
};

}

#endif