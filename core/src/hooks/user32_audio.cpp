#include "user32_internal.hpp"

#include <windows.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>

// If your project relies on a specific header for winrt::com_ptr, 
// make sure it is included here (e.g., #include "core.hpp" or <winrt/base.h>)
#include "core.hpp" 

void windower::user32::set_process_muted(bool mute) noexcept
{
    bool const com_initialized =
        SUCCEEDED(::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));

    winrt::com_ptr<::IMMDeviceEnumerator> enumerator;
    if (SUCCEEDED(
        ::CoCreateInstance(
            __uuidof(::MMDeviceEnumerator), nullptr, CLSCTX_ALL,
            IID_PPV_ARGS(enumerator.put()))))
    {
        winrt::com_ptr<::IMMDevice> device;
        if (SUCCEEDED(enumerator->GetDefaultAudioEndpoint(
            eRender, eConsole, device.put())))
        {
            winrt::com_ptr<::IAudioSessionManager> session_manager;
            if (SUCCEEDED(device->Activate(
                __uuidof(::IAudioSessionManager), CLSCTX_ALL, nullptr,
                (void**)session_manager.put())))
            {
                winrt::com_ptr<::ISimpleAudioVolume> audio_volume;
                if (SUCCEEDED(session_manager->GetSimpleAudioVolume(
                    nullptr, FALSE, audio_volume.put())))
                {
                    audio_volume->SetMute(mute, nullptr);
                }
            }
        }
    }
    if (com_initialized)
    {
        ::CoUninitialize();
    }
}
