#pragma once
#pragma once
#include <functional>
#include <string_view>

namespace windower::ui { class addon_browser; }

namespace windower::ui::session_tracker
{
    using logger_callback = std::function<void(std::u8string_view)>;

    // Monitors FFXI login state in the deep background to auto-load/unload addons
    void update(bool player_active, windower::ui::addon_browser& browser, logger_callback const& log) noexcept;
}
