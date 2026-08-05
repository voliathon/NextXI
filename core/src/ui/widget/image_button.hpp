#ifndef WINDOWER_UI_WIDGET_IMAGE_BUTTON_HPP
#define WINDOWER_UI_WIDGET_IMAGE_BUTTON_HPP

#include "ui/color.hpp"
#include "ui/context.hpp"
#include "ui/id.hpp"
#include "ui/widget/basic_button.hpp"

namespace windower::ui::widget
{

class image_button_descriptor
{
public:
    constexpr image_button_descriptor() noexcept = default;
    constexpr image_button_descriptor(
        patch const& normal, patch const& hot, patch const& active,
        patch const& disabled) noexcept :
        m_normal{normal},
        m_hot{hot}, m_active{active}, m_disabled{disabled}
    {}
    constexpr image_button_descriptor(
        patch const& normal, patch const& hot, patch const& active,
        patch const& disabled, system_cursor cursor) noexcept :
        m_normal{normal},
        m_hot{hot}, m_active{active}, m_disabled{disabled}, m_cursor{cursor}
    {}
    constexpr image_button_descriptor(
        std::u8string_view image, patch const& normal, patch const& hot,
        patch const& active, patch const& disabled) noexcept :
        m_image_data{image.data()},
        m_image_size{image.size()}, m_normal{normal}, m_hot{hot},
        m_active{active}, m_disabled{disabled}
    {}
    constexpr image_button_descriptor(
        std::u8string_view image, patch const& normal, patch const& hot,
        patch const& active, patch const& disabled,
        system_cursor cursor) noexcept :
        m_image_data{image.data()},
        m_image_size{image.size()}, m_normal{normal}, m_hot{hot},
        m_active{active}, m_disabled{disabled}, m_cursor{cursor}
    {}

    constexpr void image(std::u8string_view image)
    {
        m_image_data = image.data();
        m_image_size = image.size();
    }
    constexpr void normal(patch const& normal) { m_normal = normal; }
    constexpr void hot(patch const& hot) { m_hot = hot; }
    constexpr void active(patch const& active) { m_active = active; }
    constexpr void disabled(patch const& disabled) { m_disabled = disabled; }
    constexpr void cursor(system_cursor cursor) { m_cursor = cursor; }

    constexpr std::u8string_view image() const
    {
        return {m_image_data, m_image_size};
    }
    constexpr patch const& normal() const { return m_normal; }
    constexpr patch const& hot() const { return m_hot; }
    constexpr patch const& active() const { return m_active; }
    constexpr patch const& disabled() const { return m_disabled; }
    constexpr system_cursor cursor() const { return m_cursor; }

    constexpr patch& normal() { return m_normal; }
    constexpr patch& hot() { return m_hot; }
    constexpr patch& active() { return m_active; }
    constexpr patch& disabled() { return m_disabled; }

private:
    char8_t const* m_image_data = u8":system";
    std::size_t m_image_size    = 7;
    patch m_normal;
    patch m_hot;
    patch m_active;
    patch m_disabled;
    system_cursor m_cursor = system_cursor::hot;
};

button_state image_button(
    context& ctx, id id, image_button_descriptor const& descriptor) noexcept;

}

#endif