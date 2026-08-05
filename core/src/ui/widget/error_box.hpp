#ifndef WINDOWER_UI_WIDGET_ERROR_BOX_HPP
#define WINDOWER_UI_WIDGET_ERROR_BOX_HPP

#include "ui/context.hpp"
#include "ui/id.hpp"
#include "ui/rectangle.hpp"

#include <string_view>

namespace windower::ui::widget
{

void error_box(
    context& ctx, id id, std::u8string_view widget,
    std::u8string_view message = u8"") noexcept;

}

#endif