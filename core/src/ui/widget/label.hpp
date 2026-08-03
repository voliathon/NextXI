#ifndef WINDOWER_UI_WIDGET_LABEL_HPP
#define WINDOWER_UI_WIDGET_LABEL_HPP

#include "ui/context.hpp"

#include <string_view>

namespace windower::ui::widget
{

void label(context& ctx, std::u8string_view text) noexcept;

}

#endif