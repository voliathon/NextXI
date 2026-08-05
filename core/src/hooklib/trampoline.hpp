#ifndef WINDOWER_HOOKLIB_TRAMPOLINE_HPP
#define WINDOWER_HOOKLIB_TRAMPOLINE_HPP

#include <memory>

namespace windower::hooklib
{

class trampoline
{
public:
    trampoline() = default;
    trampoline(void (*)(), void (*)(), void (*)() = nullptr);

    explicit operator bool() const noexcept;

    void (*target() const noexcept)();

private:
    class block;

    block const* m_block;
};

}

#endif