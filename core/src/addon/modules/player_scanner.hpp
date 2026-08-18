#pragma once

namespace windower::player_scanner
{
    // Returns the JSON if the player is authenticated, nullptr if locked.
    char const* get_local_player_json() noexcept;

    // Diagnostic UI Hooks
    void trigger_manual_scan() noexcept;
    void reset_scan() noexcept;
    char const* get_diagnostic_message() noexcept;
}
