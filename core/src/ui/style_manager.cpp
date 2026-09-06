#include "ui/style_manager.hpp"
#include "utilities/paths.hpp"
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

namespace windower::ui {

    void style_manager::apply_current_styling() {
        ImGuiStyle& style = ImGui::GetStyle();

        style.Colors[ImGuiCol_Text] = ImVec4(text_col[0], text_col[1], text_col[2], 1.0f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(win_bg[0], win_bg[1], win_bg[2], 0.94f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(frame_bg[0], frame_bg[1], frame_bg[2], 1.0f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(title_bg[0], title_bg[1], title_bg[2], 1.0f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(title_act[0], title_act[1], title_act[2], 1.0f);
        style.Colors[ImGuiCol_Button] = ImVec4(btn_col[0], btn_col[1], btn_col[2], 1.0f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(btn_hov[0], btn_hov[1], btn_hov[2], 1.0f);
        style.Colors[ImGuiCol_Header] = ImVec4(header_col[0], header_col[1], header_col[2], 1.0f);
        style.Colors[ImGuiCol_Tab] = ImVec4(tab_col[0], tab_col[1], tab_col[2], 1.0f);
        style.Colors[ImGuiCol_TabActive] = ImVec4(tab_act[0], tab_act[1], tab_act[2], 1.0f);

        style.Colors[ImGuiCol_CheckMark] = ImVec4(accent_col[0], accent_col[1], accent_col[2], 1.0f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(accent_col[0], accent_col[1], accent_col[2], 1.0f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(accent_col[0], accent_col[1], accent_col[2], 1.0f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(accent_col[0], accent_col[1], accent_col[2], 1.0f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(accent_col[0], accent_col[1], accent_col[2], 1.0f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(accent_col[0], accent_col[1], accent_col[2], 1.0f);

        style.WindowRounding = window_rounding;
        style.FrameRounding = frame_rounding;
        style.TabRounding = tab_rounding;

        ImGui::GetIO().FontGlobalScale = font_scale;

        ImGuiIO& io = ImGui::GetIO();
        if (font_style_idx >= 0 && font_style_idx < io.Fonts->Fonts.Size) {
            io.FontDefault = io.Fonts->Fonts[font_style_idx];
        }
    }

    void style_manager::save_profile(const std::string& profile_name) {
        if (profile_name.empty()) return;

        auto settings_dir = windower::user_path() / u8"assets" / u8"settings";
        std::filesystem::create_directories(settings_dir);
        auto path = settings_dir / (u8"styles_" + std::u8string(profile_name.begin(), profile_name.end()) + u8".json");

        nlohmann::json j;
        j["typography"]["global_scale"] = font_scale;
        j["typography"]["style_index"] = font_style_idx;

        j["colors"]["text"] = { text_col[0], text_col[1], text_col[2] };
        j["colors"]["win_bg"] = { win_bg[0], win_bg[1], win_bg[2] };
        j["colors"]["frame_bg"] = { frame_bg[0], frame_bg[1], frame_bg[2] };
        j["colors"]["title_bg"] = { title_bg[0], title_bg[1], title_bg[2] };
        j["colors"]["title_act"] = { title_act[0], title_act[1], title_act[2] };
        j["colors"]["btn_col"] = { btn_col[0], btn_col[1], btn_col[2] };
        j["colors"]["btn_hov"] = { btn_hov[0], btn_hov[1], btn_hov[2] };
        j["colors"]["header_col"] = { header_col[0], header_col[1], header_col[2] };
        j["colors"]["tab_col"] = { tab_col[0], tab_col[1], tab_col[2] };
        j["colors"]["tab_act"] = { tab_act[0], tab_act[1], tab_act[2] };
        j["colors"]["accent_col"] = { accent_col[0], accent_col[1], accent_col[2] };

        j["geometry"]["window_rounding"] = window_rounding;
        j["geometry"]["frame_rounding"] = frame_rounding;
        j["geometry"]["tab_rounding"] = tab_rounding;

        std::ofstream file(path);
        if (file.is_open()) {
            file << j.dump(4);
        }
    }

    void style_manager::load_profile(const std::string& profile_name) {
        auto settings_dir = windower::user_path() / u8"assets" / u8"settings";
        auto path = settings_dir / (u8"styles_" + std::u8string(profile_name.begin(), profile_name.end()) + u8".json");

        if (!std::filesystem::exists(path)) {
            path = settings_dir / u8"styles_global.json";
            if (!std::filesystem::exists(path)) return;
        }

        try {
            std::ifstream file(path);
            nlohmann::json j;
            file >> j;

            if (j.contains("typography")) {
                font_scale = j["typography"].value("global_scale", 1.0f);
                font_style_idx = j["typography"].value("style_index", 0);
            }

            auto load_color = [&](const char* key, float* out_arr) {
                if (j.contains("colors") && j["colors"].contains(key) && j["colors"][key].is_array() && j["colors"][key].size() >= 3) {
                    out_arr[0] = j["colors"][key][0].get<float>();
                    out_arr[1] = j["colors"][key][1].get<float>();
                    out_arr[2] = j["colors"][key][2].get<float>();
                }
                };

            load_color("text", text_col);
            load_color("win_bg", win_bg);
            load_color("frame_bg", frame_bg);
            load_color("title_bg", title_bg);
            load_color("title_act", title_act);
            load_color("btn_col", btn_col);
            load_color("btn_hov", btn_hov);
            load_color("header_col", header_col);
            load_color("tab_col", tab_col);
            load_color("tab_act", tab_act);
            load_color("accent_col", accent_col);

            if (j.contains("geometry")) {
                window_rounding = j["geometry"].value("window_rounding", 4.0f);
                frame_rounding = j["geometry"].value("frame_rounding", 3.0f);
                tab_rounding = j["geometry"].value("tab_rounding", 3.0f);
            }

            apply_current_styling();
        }
        catch (...) {}
    }

    void style_manager::apply_preset(int preset_id) {
        ImGui::StyleColorsDark();

        switch (preset_id) {
        case 0: // PlayOnline
            text_col[0] = 0.15f; text_col[1] = 0.10f; text_col[2] = 0.05f;
            win_bg[0] = 0.90f; win_bg[1] = 0.85f; win_bg[2] = 0.75f;
            frame_bg[0] = 0.95f; frame_bg[1] = 0.92f; frame_bg[2] = 0.85f;
            title_bg[0] = 0.75f; title_bg[1] = 0.65f; title_bg[2] = 0.50f;
            title_act[0] = 0.85f; title_act[1] = 0.75f; title_act[2] = 0.60f;
            btn_col[0] = 0.80f; btn_col[1] = 0.70f; btn_col[2] = 0.55f;
            btn_hov[0] = 0.85f; btn_hov[1] = 0.78f; btn_hov[2] = 0.65f;
            header_col[0] = 0.82f; header_col[1] = 0.72f; header_col[2] = 0.58f;
            tab_col[0] = 0.75f; tab_col[1] = 0.65f; tab_col[2] = 0.50f;
            tab_act[0] = 0.90f; tab_act[1] = 0.85f; tab_act[2] = 0.75f;
            accent_col[0] = 0.50f; accent_col[1] = 0.20f; accent_col[2] = 0.15f;
            window_rounding = 2.0f; frame_rounding = 2.0f; tab_rounding = 4.0f;
            break;
        case 1: // Warm Tans
            text_col[0] = 0.05f; text_col[1] = 0.15f; text_col[2] = 0.30f;
            win_bg[0] = 0.75f; win_bg[1] = 0.80f; win_bg[2] = 0.85f;
            frame_bg[0] = 0.85f; frame_bg[1] = 0.90f; frame_bg[2] = 0.95f;
            title_bg[0] = 0.15f; title_bg[1] = 0.25f; title_bg[2] = 0.45f;
            title_act[0] = 0.20f; title_act[1] = 0.35f; title_act[2] = 0.60f;
            btn_col[0] = 0.50f; btn_col[1] = 0.60f; btn_col[2] = 0.75f;
            btn_hov[0] = 0.60f; btn_hov[1] = 0.70f; btn_hov[2] = 0.85f;
            header_col[0] = 0.40f; header_col[1] = 0.55f; header_col[2] = 0.75f;
            tab_col[0] = 0.55f; tab_col[1] = 0.65f; tab_col[2] = 0.80f;
            tab_act[0] = 0.75f; tab_act[1] = 0.80f; tab_act[2] = 0.85f;
            accent_col[0] = 0.10f; accent_col[1] = 0.25f; accent_col[2] = 0.60f;
            window_rounding = 8.0f; frame_rounding = 6.0f; tab_rounding = 6.0f;
            break;
        case 2: // Nord Frost
            text_col[0] = 0.90f; text_col[1] = 0.92f; text_col[2] = 0.95f;
            win_bg[0] = 0.18f; win_bg[1] = 0.20f; win_bg[2] = 0.25f;
            frame_bg[0] = 0.23f; frame_bg[1] = 0.26f; frame_bg[2] = 0.32f;
            title_bg[0] = 0.23f; title_bg[1] = 0.26f; title_bg[2] = 0.32f;
            title_act[0] = 0.33f; title_act[1] = 0.38f; title_act[2] = 0.46f;
            btn_col[0] = 0.33f; btn_col[1] = 0.38f; btn_col[2] = 0.46f;
            btn_hov[0] = 0.43f; btn_hov[1] = 0.49f; btn_hov[2] = 0.58f;
            header_col[0] = 0.33f; header_col[1] = 0.38f; header_col[2] = 0.46f;
            tab_col[0] = 0.23f; tab_col[1] = 0.26f; tab_col[2] = 0.32f;
            tab_act[0] = 0.33f; tab_act[1] = 0.38f; tab_act[2] = 0.46f;
            accent_col[0] = 0.53f; accent_col[1] = 0.75f; accent_col[2] = 0.82f;
            window_rounding = 6.0f; frame_rounding = 4.0f; tab_rounding = 4.0f;
            break;
        case 3: // Deep Purple
            text_col[0] = 0.85f; text_col[1] = 0.90f; text_col[2] = 0.95f;
            win_bg[0] = 0.05f; win_bg[1] = 0.02f; win_bg[2] = 0.08f;
            frame_bg[0] = 0.10f; frame_bg[1] = 0.05f; frame_bg[2] = 0.15f;
            title_bg[0] = 0.15f; title_bg[1] = 0.05f; title_bg[2] = 0.25f;
            title_act[0] = 0.30f; title_act[1] = 0.10f; title_act[2] = 0.50f;
            btn_col[0] = 0.20f; btn_col[1] = 0.10f; btn_col[2] = 0.35f;
            btn_hov[0] = 0.35f; btn_hov[1] = 0.15f; btn_hov[2] = 0.60f;
            header_col[0] = 0.25f; header_col[1] = 0.10f; header_col[2] = 0.40f;
            tab_col[0] = 0.15f; tab_col[1] = 0.05f; tab_col[2] = 0.25f;
            tab_act[0] = 0.40f; tab_act[1] = 0.15f; tab_act[2] = 0.65f;
            accent_col[0] = 0.10f; accent_col[1] = 0.90f; accent_col[2] = 0.70f;
            window_rounding = 12.0f; frame_rounding = 8.0f; tab_rounding = 8.0f;
            break;
        case 4: // Navy Blue
            text_col[0] = 0.90f; text_col[1] = 0.90f; text_col[2] = 0.95f;
            win_bg[0] = 0.05f; win_bg[1] = 0.02f; win_bg[2] = 0.02f;
            frame_bg[0] = 0.10f; frame_bg[1] = 0.02f; frame_bg[2] = 0.02f;
            title_bg[0] = 0.25f; title_bg[1] = 0.02f; title_bg[2] = 0.02f;
            title_act[0] = 0.60f; title_act[1] = 0.05f; title_act[2] = 0.05f;
            btn_col[0] = 0.35f; btn_col[1] = 0.05f; btn_col[2] = 0.05f;
            btn_hov[0] = 0.50f; btn_hov[1] = 0.10f; btn_hov[2] = 0.10f;
            header_col[0] = 0.40f; header_col[1] = 0.05f; header_col[2] = 0.05f;
            tab_col[0] = 0.15f; tab_col[1] = 0.02f; tab_col[2] = 0.02f;
            tab_act[0] = 0.45f; tab_act[1] = 0.05f; tab_act[2] = 0.05f;
            accent_col[0] = 1.00f; accent_col[1] = 0.20f; accent_col[2] = 0.10f;
            window_rounding = 2.0f; frame_rounding = 2.0f; tab_rounding = 2.0f;
            break;
        case 5: // Ember Red
            text_col[0] = 0.95f; text_col[1] = 0.90f; text_col[2] = 0.90f;
            win_bg[0] = 0.02f; win_bg[1] = 0.02f; win_bg[2] = 0.05f;
            frame_bg[0] = 0.02f; frame_bg[1] = 0.02f; frame_bg[2] = 0.10f;
            title_bg[0] = 0.02f; title_bg[1] = 0.02f; title_bg[2] = 0.25f;
            title_act[0] = 0.05f; title_act[1] = 0.05f; title_act[2] = 0.60f;
            btn_col[0] = 0.05f; btn_col[1] = 0.05f; btn_col[2] = 0.35f;
            btn_hov[0] = 0.10f; btn_hov[1] = 0.10f; btn_hov[2] = 0.50f;
            header_col[0] = 0.05f; header_col[1] = 0.05f; header_col[2] = 0.40f;
            tab_col[0] = 0.02f; tab_col[1] = 0.02f; tab_col[2] = 0.15f;
            tab_act[0] = 0.05f; tab_act[1] = 0.05f; tab_act[2] = 0.45f;
            accent_col[0] = 0.10f; accent_col[1] = 0.20f; accent_col[2] = 1.00f;
            window_rounding = 2.0f; frame_rounding = 2.0f; tab_rounding = 2.0f;
            break;
        case 6: // Cyber Neon
            text_col[0] = 0.95f; text_col[1] = 0.95f; text_col[2] = 0.95f;
            win_bg[0] = 0.05f; win_bg[1] = 0.05f; win_bg[2] = 0.08f;
            frame_bg[0] = 0.10f; frame_bg[1] = 0.10f; frame_bg[2] = 0.14f;
            title_bg[0] = 0.12f; title_bg[1] = 0.08f; title_bg[2] = 0.18f;
            title_act[0] = 0.60f; title_act[1] = 0.10f; title_act[2] = 0.50f;
            btn_col[0] = 0.15f; btn_col[1] = 0.10f; btn_col[2] = 0.22f;
            btn_hov[0] = 0.80f; btn_hov[1] = 0.15f; btn_hov[2] = 0.65f;
            header_col[0] = 0.20f; header_col[1] = 0.12f; header_col[2] = 0.30f;
            tab_col[0] = 0.10f; tab_col[1] = 0.08f; tab_col[2] = 0.15f;
            tab_act[0] = 0.70f; tab_act[1] = 0.12f; tab_act[2] = 0.55f;
            accent_col[0] = 0.00f; accent_col[1] = 0.90f; accent_col[2] = 0.95f;
            window_rounding = 0.0f; frame_rounding = 0.0f; tab_rounding = 0.0f;
            break;
        case 7: // Jeuno Marble
            text_col[0] = 0.10f; text_col[1] = 0.10f; text_col[2] = 0.10f;
            win_bg[0] = 0.95f; win_bg[1] = 0.95f; win_bg[2] = 0.95f;
            frame_bg[0] = 1.00f; frame_bg[1] = 1.00f; frame_bg[2] = 1.00f;
            title_bg[0] = 0.80f; title_bg[1] = 0.80f; title_bg[2] = 0.80f;
            title_act[0] = 0.90f; title_act[1] = 0.85f; title_act[2] = 0.75f;
            btn_col[0] = 0.85f; btn_col[1] = 0.85f; btn_col[2] = 0.85f;
            btn_hov[0] = 0.92f; btn_hov[1] = 0.92f; btn_hov[2] = 0.92f;
            header_col[0] = 0.88f; header_col[1] = 0.88f; header_col[2] = 0.88f;
            tab_col[0] = 0.80f; tab_col[1] = 0.80f; tab_col[2] = 0.80f;
            tab_act[0] = 0.95f; tab_act[1] = 0.95f; tab_act[2] = 0.95f;
            accent_col[0] = 0.70f; accent_col[1] = 0.50f; accent_col[2] = 0.15f;
            window_rounding = 4.0f; frame_rounding = 4.0f; tab_rounding = 4.0f;
            break;
        }
        apply_current_styling();
    }
}
