#ifndef WINDOWER_CLOAK_HPP
#define WINDOWER_CLOAK_HPP

namespace windower
{

class cloak_guard;

void pin_and_cloak() noexcept;
[[nodiscard]] cloak_guard uncloak() noexcept;

class cloak_guard final
{
public:
    cloak_guard(cloak_guard const&) = delete;
    cloak_guard(cloak_guard&&)      = default;

    ~cloak_guard();

    cloak_guard& operator=(cloak_guard const&) = delete;
    cloak_guard& operator=(cloak_guard&&)      = delete;

private:
    cloak_guard() = default;

    friend cloak_guard uncloak() noexcept;
};

}

#endif