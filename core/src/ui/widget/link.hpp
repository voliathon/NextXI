#ifndef WINDOWER_UI_WIDGET_LINK_HPP
#define WINDOWER_UI_WIDGET_LINK_HPP

#include "ui/context.hpp"
#include "ui/id.hpp"
#include "ui/widget/basic_button.hpp"

#include <string_view>

namespace windower::ui::widget
{

button_state link(context& ctx, id id, std::u8string_view text) noexcept;

}

#endif