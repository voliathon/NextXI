#ifndef WINDOWER_UI_MOUSE_HPP
#define WINDOWER_UI_MOUSE_HPP

#include "ui/mouse_button.hpp"
#include "ui/mouse_scroll_axis.hpp"
#include "ui/vector.hpp"

#include <array>
#include <cstdint>

namespace windower::ui
{

class context;

class mouse
{
public:
    vector const& position() const noexcept;

    bool was_pressed(mouse_button button) const noexcept;
    bool was_released(mouse_button button) const noexcept;
    bool is_held(mouse_button button) const noexcept;
    std::uint64_t hold_time(mouse_button button) const noexcept;
    vector const& scroll_offset() const noexcept;

    void move(vector position) noexcept;
    void press(mouse_button button) noexcept;
    void release(mouse_button button) noexcept;
    void scroll(vector distance) noexcept;

    void update() noexcept;

private:
    vector m_position;
    std::array<std::int64_t, 5> m_buttons = {};
    vector m_scroll                       = {};
    std::uint64_t m_time                  = 0;
    std::int32_t m_last_click_x           = 0;
    std::int32_t m_last_click_y           = 0;
    std::uint64_t m_last_click_time       = 0;
    std::int8_t m_last_click_button       = 0;
    std::int8_t m_click_count             = 0;
};

}

#endif