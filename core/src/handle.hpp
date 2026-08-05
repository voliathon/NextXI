#ifndef WINDOWER_HANDLE_HPP
#define WINDOWER_HANDLE_HPP

namespace windower
{

class handle
{
public:
    handle() noexcept;
    handle(handle const&);
    handle(handle&&) noexcept;
    handle(void* handle) noexcept;

    ~handle();

    handle& operator=(handle const&) noexcept;
    handle& operator=(handle&&) noexcept;
    explicit operator bool() const noexcept;
    operator void*() const noexcept;

private:
    void* m_handle;
};

}

#endif