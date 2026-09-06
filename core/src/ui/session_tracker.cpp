#include "session_tracker.hpp"
#include "core.hpp"
#include "addon/addon_manager.hpp"
#include "command_manager.hpp"
#include "addon/modules/player_scanner.hpp"
#include "ui/addon_browser.hpp" 
#include <string>
#include <chrono>

namespace windower::ui::session_tracker
{
    void update(bool player_active, windower::ui::addon_browser& browser, logger_callback const& log) noexcept
    {
        static bool s_was_player_active = false;
        static bool s_pending_autoload = false;
        static std::string s_pending_name = "";

        static std::chrono::steady_clock::time_point s_login_time;

        // ----------------------------------------------------
        // Transition: Logged Out
        // ----------------------------------------------------
        if (s_was_player_active && !player_active) {
            log(u8"--- SESSION ENDED: Unloading all active addons ---");

            s_pending_autoload = false;

            if (core::instance().addon_manager) {
                for (auto const& a : browser.get_cached_addons()) {
                    if (core::instance().addon_manager->get(a.name)) {
                        core::instance().run_on_next_frame([cmd = u8"//unload " + a.name]() {
                            command_manager::instance().handle_command(cmd, command_source::console);
                            });
                    }
                }
            }
        }
        // ----------------------------------------------------
        // Transition: Logged In
        // ----------------------------------------------------
        else if (!s_was_player_active && player_active) {
            char const* name_ptr = windower::player_scanner::get_cached_player_name();
            if (name_ptr && name_ptr[0] != '\0') {

                s_pending_autoload = true;
                s_pending_name = name_ptr;
                s_login_time = std::chrono::steady_clock::now();

                log(u8"--- SESSION STARTED: Waiting 3 seconds for engine to stabilize... ---");
            }
        }

        // ----------------------------------------------------
        // Safely execute the Autoload after a 3-second delay
        // ----------------------------------------------------
        if (s_pending_autoload && player_active) {
            if (std::chrono::steady_clock::now() - s_login_time > std::chrono::seconds(3)) {

                s_pending_autoload = false;

                std::u8string u8_name(s_pending_name.begin(), s_pending_name.end());
                log(u8"--- Engine Stable: Auto-loading profile for " + u8_name + u8" ---");

                core::instance().run_on_next_frame([name = s_pending_name]() {
                    addon_browser::run_autoload(name);
                    });
            }
        }

        s_was_player_active = player_active;
    }
}
