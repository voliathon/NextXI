#ifndef WINDOWER_WRAPPERS_DIRECT_INPUT_HPP
#define WINDOWER_WRAPPERS_DIRECT_INPUT_HPP

#include <windows.h>
#include <dinput.h>

namespace windower
{

class direct_input final : public ::IDirectInput8A
{
public:
    direct_input() = delete;
    direct_input(::IDirectInput8A*) noexcept;
    direct_input(direct_input const&) = delete;
    direct_input(direct_input&&) = delete;
    ~direct_input();

    direct_input& operator=(direct_input const&) = delete;
    direct_input& operator=(direct_input&&) = delete;
    ::HRESULT STDMETHODCALLTYPE
    QueryInterface(::IID const&, void**) noexcept override;
    ::ULONG STDMETHODCALLTYPE AddRef() noexcept override;
    ::ULONG STDMETHODCALLTYPE Release() noexcept override;
    ::HRESULT STDMETHODCALLTYPE CreateDevice(
        ::GUID const&, ::IDirectInputDevice8A**, ::IUnknown*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE EnumDevices(
        ::DWORD, ::LPDIENUMDEVICESCALLBACKA, ::LPVOID,
        ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    GetDeviceStatus(::GUID const&) noexcept override;
    ::HRESULT
        STDMETHODCALLTYPE RunControlPanel(::HWND, ::DWORD) noexcept override;
    ::HRESULT
        STDMETHODCALLTYPE Initialize(::HINSTANCE, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE
    FindDevice(::GUID const&, ::CHAR const*, ::GUID*) noexcept override;
    ::HRESULT STDMETHODCALLTYPE EnumDevicesBySemantics(
        ::CHAR const*, ::DIACTIONFORMATA*, ::LPDIENUMDEVICESBYSEMANTICSCBA,
        void*, ::DWORD) noexcept override;
    ::HRESULT STDMETHODCALLTYPE ConfigureDevices(
        ::LPDICONFIGUREDEVICESCALLBACK, ::DICONFIGUREDEVICESPARAMSA*, ::DWORD,
        void*) noexcept override;

private:
    ::IDirectInput8A* m_impl;
    ::ULONG m_count = 0;
};

}

#endif