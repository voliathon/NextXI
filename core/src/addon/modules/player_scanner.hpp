#pragma once

namespace windower::player_scanner
{
    // Returns the JSON if the player is authenticated, nullptr if locked.
    char const* get_local_player_json() noexcept;

    // Returns the raw string of the local player's name
    char const* get_cached_player_name() noexcept;

    // Diagnostic UI Hooks
    void trigger_manual_scan() noexcept;
    void reset_scan() noexcept;
    char const* get_diagnostic_message() noexcept;
}
