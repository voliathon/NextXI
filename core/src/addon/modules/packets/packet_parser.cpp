#include "packet_parser.hpp"
#include "player_state.hpp"
#include <cstring>

namespace windower::network {
    void parse_incoming_packet(uint16_t id, std::span<std::byte const> data) {
        if (data.size() < 4) return;
        const uint8_t* raw = reinterpret_cast<const uint8_t*>(data.data());

        switch (id) {
        case 0x0DF: { // Char Update (Current HP/MP/TP)
            if (data.size() >= 0x10) {
                uint32_t pid, hp, mp, tp;
                std::memcpy(&pid, raw + 0x00, 4); // Offset 0 is actually Player ID
                std::memcpy(&hp, raw + 0x04, 4);
                std::memcpy(&mp, raw + 0x08, 4);
                std::memcpy(&tp, raw + 0x0C, 4);
                player_state_manager::get().update_current_vitals(pid, hp, mp, tp);
            }
            break;
        }
        case 0x061: { // Char Stats (Max HP/MP)
            if (data.size() >= 0x08) {
                uint32_t max_hp, max_mp;
                std::memcpy(&max_hp, raw + 0x00, 4);
                std::memcpy(&max_mp, raw + 0x04, 4);
                player_state_manager::get().update_max_vitals(max_hp, max_mp);
            }
            break;
        }
        case 0x00A: { // Zone In
            if (data.size() >= 0x2E) {
                uint16_t zone;
                // Standard Windower offset is 0x30. Minus 4-byte header = 0x2C
                std::memcpy(&zone, raw + 0x2C, 2);
                player_state_manager::get().update_zone(zone);
            }
            break;
        }
        }
    }
}
