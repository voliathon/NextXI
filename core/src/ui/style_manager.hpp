#pragma once

#include <string>

namespace windower::ui {

    class style_manager {
    public:
        // Singleton access
        static style_manager& instance() {
            static style_manager s_instance;
            return s_instance;
        }

        // Core Actions
        void load_profile(const std::string& profile_name);
        void save_profile(const std::string& profile_name);
        void apply_preset(int preset_id);
        void apply_current_styling();

        // Tracker
        std::string last_profile = "UNINITIALIZED";

        // Typography State
        float font_scale = 1.0f;
        int font_style_idx = 0;

        // Color State
        float text_col[3] = { 1.0f, 1.0f, 1.0f };
        float win_bg[3] = { 0.06f, 0.06f, 0.06f };
        float frame_bg[3] = { 0.12f, 0.12f, 0.12f };
        float title_bg[3] = { 0.08f, 0.08f, 0.08f };
        float title_act[3] = { 0.20f, 0.25f, 0.35f };
        float btn_col[3] = { 0.20f, 0.22f, 0.27f };
        float btn_hov[3] = { 0.30f, 0.35f, 0.45f };
        float header_col[3] = { 0.25f, 0.25f, 0.30f };
        float tab_col[3] = { 0.15f, 0.15f, 0.18f };
        float tab_act[3] = { 0.25f, 0.35f, 0.55f };
        float accent_col[3] = { 0.35f, 0.55f, 0.85f };

        // Geometry State
        float window_rounding = 4.0f;
        float frame_rounding = 3.0f;
        float tab_rounding = 3.0f;

    private:
        style_manager() = default;
        ~style_manager() = default;
    };

}
