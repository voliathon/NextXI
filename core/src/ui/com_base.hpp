#ifndef WINDOWER_UI_COM_BASE_HPP
#define WINDOWER_UI_COM_BASE_HPP

#include <unknwn.h>

#include <atomic>
#include <type_traits>

namespace windower::ui
{

template<typename T>
class com_base : public T
{
    static_assert(std::is_base_of_v<::IUnknown, T>);

public:
    com_base(com_base const&) = delete;
    com_base(com_base&&) = delete;

    virtual ~com_base() noexcept = default;

    com_base operator=(com_base const&) = delete;
    com_base operator=(com_base&&) = delete;
    ::HRESULT STDMETHODCALLTYPE
    QueryInterface(REFIID riid, void** ppvObject) noexcept override = 0;

    ::ULONG STDMETHODCALLTYPE AddRef() noexcept final { return ++m_ref_count; }

    ::ULONG STDMETHODCALLTYPE Release() noexcept final
    {
        auto const ref_count = --m_ref_count;
        if (ref_count == 0)
        {
            delete this;
        }
        return ref_count;
    }

protected:
    com_base() = default;

private:
    std::atomic<::ULONG> m_ref_count = 1;
};

template<typename T>
::HRESULT STDMETHODCALLTYPE
com_base<T>::QueryInterface(REFIID riid, void** ppvObject) noexcept
{
    if (!ppvObject)
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if (::IsEqualGUID(riid, ::IID_IUnknown))
    {
        ::IUnknown* const result = this;
        *ppvObject = result;
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

}

#endif