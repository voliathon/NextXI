#pragma once
#include <cstdint>
#include <mutex>

namespace windower::network {
    struct player_data {
        uint32_t id = 0;
        uint16_t index = 0;
        uint16_t zone_id = 0;
        uint32_t hp = 0;
        uint32_t max_hp = 0;
        uint32_t mp = 0;
        uint32_t max_mp = 0;
        uint32_t tp = 0;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    class player_state_manager {
    public:
        static player_state_manager& get() {
            static player_state_manager instance;
            return instance;
        }

        void update_current_vitals(uint32_t id, uint32_t hp, uint32_t mp, uint32_t tp) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_data.id = id;
            m_data.hp = hp;
            m_data.mp = mp;
            m_data.tp = tp;
        }

        void update_max_vitals(uint32_t max_hp, uint32_t max_mp) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_data.max_hp = max_hp;
            m_data.max_mp = max_mp;
        }

        void update_zone(uint16_t zone_id) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_data.zone_id = zone_id;
        }

        player_data get_data() {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_data;
        }

    private:
        player_state_manager() = default;
        player_data m_data;
        std::mutex m_mutex;
    };
}
