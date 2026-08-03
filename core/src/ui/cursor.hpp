#ifndef WINDOWER_UI_CURSOR_HPP
#define WINDOWER_UI_CURSOR_HPP

#include <filesystem>

namespace windower::ui
{

class context;

class cursor
{
public:
    cursor() noexcept = default;
    cursor(cursor const& other) noexcept;
    cursor(cursor&& other) noexcept;
    explicit cursor(std::filesystem::path const& path) noexcept;

    ~cursor() noexcept;

    cursor& operator=(cursor const& other) noexcept;
    cursor& operator=(cursor&& other) noexcept;
    explicit operator bool() const noexcept;

private:
    void* m_handle = nullptr;

    friend class context;
};

}

#endif