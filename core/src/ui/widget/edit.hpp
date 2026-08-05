#ifndef WINDOWER_UI_WIDGET_EDIT_HPP
#define WINDOWER_UI_WIDGET_EDIT_HPP

#include "ui/context.hpp"
#include "ui/id.hpp"

#include <string>
#include <string_view>

namespace windower::ui::widget
{

class edit_state;

void basic_edit(context&, id, edit_state&) noexcept;

class edit_state
{
public:
    void text(std::u8string_view) noexcept;
    std::u8string_view text() const noexcept;

private:
    char8_t const* text_data = nullptr;
    std::size_t text_size   = 0;
    bool text_changed       = false;

    friend void basic_edit(context&, id, edit_state&) noexcept;
};

void edit(context&, id, edit_state&) noexcept;

}

#endif