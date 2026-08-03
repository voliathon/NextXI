#ifndef WINDOWER_UI_WIDGET_CHECK_HPP
#define WINDOWER_UI_WIDGET_CHECK_HPP

#include "ui/context.hpp"
#include "ui/id.hpp"
#include "ui/widget/basic_button.hpp"

#include <optional>
#include <string_view>

namespace windower::ui::widget
{

button_state check(
    context& ctx, id id, std::u8string_view text,
    std::optional<bool> checked) noexcept;

}

#endif