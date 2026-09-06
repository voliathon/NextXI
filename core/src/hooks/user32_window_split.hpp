#pragma once
#include <windows.h>
#include <string>
#include <string_view>
#include <array>
#include <span>
#include <bit>
#include <memory>
#include <gsl/gsl>

namespace windower::user32
{
    struct window_data
    {
        ::WNDPROC wnd_proc = nullptr;
        ::LONG wnd_proc_a = 0;
        ::LONG wnd_proc_w = 0;
        ::MSG current = {};
        bool update_title = false;
        std::basic_string<::WCHAR> title;
    };

    constexpr auto window_data_size = (sizeof(window_data*) + alignof(::LONG) - 1) / alignof(::LONG) * alignof(::LONG);

    extern ::ATOM ffxi_class_atom;
    extern ::ATOM pol_class_atom;

    // Caches to feed the process its own windows
    extern ::HWND pol_hwnd_cache;
    extern ::HWND ffxi_hwnd_cache;

    window_data* get_window_data(::HWND hwnd) noexcept;
    void set_window_data(::HWND hwnd, window_data* ptr) noexcept;
    ::LONG set_window_wndproc(::HWND hwnd, ::LONG wnd_proc) noexcept;

    bool check_class(::HWND hwnd, ATOM expected) noexcept;
    void set_window_properties(::HWND hwnd);
    ::LRESULT CALLBACK ffxi_wnd_proc(::HWND hwnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam) noexcept;

    std::basic_string_view<::CHAR> get_atom_name(::ATOM atom, std::span<::CHAR> const buffer) noexcept;
    std::basic_string_view<::WCHAR> get_atom_name(::ATOM atom, std::span<::WCHAR> const buffer) noexcept;

    template<typename T, std::size_t N>
    bool check_class_name(T const* ptr, T const (&class_name)[N]) noexcept
    {
        auto value = std::bit_cast<std::uintptr_t>(ptr);
        if (value != 0 && value <= 0xFFFF)
        {
            std::array<T, 256> buffer{};
            auto const atom = gsl::narrow_cast<::ATOM>(value);
            auto const name = get_atom_name(atom, buffer);
            return name == class_name;
        }
        return std::basic_string_view<T>{ptr} == class_name;
    }

    void install_window_class(); void uninstall_window_class() noexcept;
    void install_window_msg();   void uninstall_window_msg() noexcept;
    void install_window_create(); void uninstall_window_create() noexcept;
}
