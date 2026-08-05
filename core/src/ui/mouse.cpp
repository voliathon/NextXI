#include "ui/mouse.hpp"

#include "ui/context.hpp"
#include "ui/vector.hpp"
#include "utility.hpp"

#include <windows.h>

#include <gsl/gsl>

#include <cstdint>

namespace windower::ui
{
namespace
{

constexpr std::uint64_t pressed  = 0x40000000'00000000;
constexpr std::uint64_t released = 0x80000000'00000000;
constexpr std::uint64_t held     = 0x20000000'00000000;

constexpr std::uint64_t state_mask = 0xE0000000'00000000;
constexpr std::uint64_t time_mask  = 0x1FFFFFFF'FFFFFFFF;

}

vector const& mouse::position() const noexcept { return m_position; }

bool mouse::was_pressed(mouse_button button) const noexcept
{
    return (gsl::at(m_buttons, to_underlying(button)) & pressed) != 0;
}

bool mouse::was_released(mouse_button button) const noexcept
{
    return (gsl::at(m_buttons, to_underlying(button)) & released) != 0;
}

bool mouse::is_held(mouse_button button) const noexcept
{
    return (gsl::at(m_buttons, to_underlying(button)) & held) != 0;
}

std::uint64_t mouse::hold_time(mouse_button button) const noexcept
{
    auto const data = gsl::at(m_buttons, to_underlying(button));
    return (m_time - (data & time_mask)) * ((data & held) != 0);
}

vector const& mouse::scroll_offset() const noexcept { return m_scroll; }

void mouse::move(vector position) noexcept { m_position = position; }

void mouse::press(mouse_button button) noexcept
{
    auto const point = ::POINT{
        gsl::narrow_cast<::LONG>(m_position.x),
        gsl::narrow_cast<::LONG>(m_position.y)};

    auto double_click_rect =
        ::RECT{m_last_click_x, m_last_click_y, m_last_click_x, m_last_click_y};
    auto const w = ::GetSystemMetrics(SM_CXDOUBLECLK) / 2;
    auto const h = ::GetSystemMetrics(SM_CYDOUBLECLK) / 2;
    ::InflateRect(&double_click_rect, w, h);

    auto const index = to_underlying(button);

    if (index != m_last_click_button ||
        !::PtInRect(&double_click_rect, point) ||
        m_time - m_last_click_time > ::GetDoubleClickTime())
    {
        m_click_count = 0;
    }

    ++m_click_count;
    m_last_click_button = index;
    m_last_click_time   = m_time;

    m_last_click_x = point.x;
    m_last_click_y = point.y;

    auto& data = gsl::at(m_buttons, index);
    data &= ~time_mask;
    data |= m_time | pressed | held;
}

void mouse::release(mouse_button button) noexcept
{
    auto& data = gsl::at(m_buttons, to_underlying(button));
    data &= ~held;
    data |= released;
}

void mouse::scroll(vector distance) noexcept { m_scroll += distance; }

void mouse::update() noexcept
{
    namespace range = std::ranges;

    m_time = ::GetTickCount64() & time_mask;
    range::transform(m_buttons, m_buttons.begin(), [](auto data) {
        return data & ~(pressed | released);
    });
    m_scroll = {};
}

}