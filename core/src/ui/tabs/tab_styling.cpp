#include "tab_styling.hpp"
#include "ui/style_manager.hpp"
#include "addon/modules/player_scanner.hpp"
#include <imgui.h>

namespace windower::ui::tabs {

    void render_styling_tab() {
        // Grab the central manager
        auto& sm = style_manager::instance();

        // 1. Profile Swap Detection
        const char* name_ptr = windower::player_scanner::get_cached_player_name();
        std::string current_profile = (name_ptr && name_ptr[0] != '\0') ? name_ptr : "global";

        if (sm.last_profile != current_profile) {
            sm.last_profile = current_profile;
            sm.load_profile(current_profile); // Load JSON when characters swap
        }

        // 2. Render the UI
        if (ImGui::BeginTabBar("StylingSubTabs")) {

            if (ImGui::BeginTabItem("Presets")) {
                ImGui::Spacing();
                ImGui::Text("Quick Visual Presets:");
                ImGui::Spacing();

                auto apply_and_save = [&](int id) {
                    sm.apply_preset(id);
                    sm.save_profile(current_profile);
                    };

                if (ImGui::Button("PlayOnline", ImVec2(180, 0))) apply_and_save(0);
                ImGui::SameLine(); ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.5f, 1.0f), "Silver & Light Blue");

                if (ImGui::Button("Warm Tans", ImVec2(180, 0))) apply_and_save(1);
                ImGui::SameLine(); ImGui::TextColored(ImVec4(0.5f, 0.6f, 0.8f, 1.0f), "Classic FFXI Tan & Brown");

                if (ImGui::Button("Nord Frost", ImVec2(180, 0))) apply_and_save(2);
                ImGui::SameLine(); ImGui::TextColored(ImVec4(0.6f, 0.8f, 0.9f, 1.0f), "Modern Cool Grey");

                if (ImGui::Button("Deep Purple", ImVec2(180, 0))) apply_and_save(3);
                ImGui::SameLine(); ImGui::TextColored(ImVec4(0.7f, 0.3f, 0.9f, 1.0f), "Dark Purples & Teal");

                if (ImGui::Button("Navy Blue", ImVec2(180, 0))) apply_and_save(4);
                ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Navy Blue & Black");

                if (ImGui::Button("Ember Red", ImVec2(180, 0))) apply_and_save(5);
                ImGui::SameLine(); ImGui::TextColored(ImVec4(0.4f, 0.3f, 0.6f, 1.0f), "Pure Red & Black");

                if (ImGui::Button("Cyber Neon", ImVec2(180, 0))) apply_and_save(6);
                ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.8f, 1.0f), "Neon Colors");

                if (ImGui::Button("Jeuno Marble", ImVec2(180, 0))) apply_and_save(7);
                ImGui::SameLine(); ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Clean White & Silver");

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button("Reset to Default Dark", ImVec2(180, 0))) {
                    ImGui::StyleColorsDark();
                    sm.save_profile(current_profile);
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Typography")) {
                ImGui::Spacing();

                if (ImGui::SliderFloat("Global Font Size", &sm.font_scale, 0.5f, 2.5f, "%.2fx Scale")) {
                    ImGui::GetIO().FontGlobalScale = sm.font_scale;
                }
                // Safely save only when dragging stops
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    sm.save_profile(current_profile);
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGuiIO& io = ImGui::GetIO();
                if (io.Fonts->Fonts.Size > 0) {
                    ImFont* current_font = io.Fonts->Fonts[sm.font_style_idx];
                    const char* preview_name = current_font->GetDebugName();
                    if (!preview_name) preview_name = "Default";

                    if (ImGui::BeginCombo("Font Style", preview_name)) {
                        for (int i = 0; i < io.Fonts->Fonts.Size; i++) {
                            ImFont* font = io.Fonts->Fonts[i];
                            const char* font_name = font->GetDebugName();
                            if (!font_name) font_name = "Default";

                            bool is_selected = (sm.font_style_idx == i);
                            if (ImGui::Selectable(font_name, is_selected)) {
                                sm.font_style_idx = i;
                                io.FontDefault = font;
                                sm.save_profile(current_profile);
                            }
                            if (is_selected) ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }
                }

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Drop .ttf files into NextXI/assets/fonts/ to add them here.");

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Color Palette")) {
                ImGui::Spacing();
                ImGui::ColorEdit3("Text Color", sm.text_col);
                ImGui::ColorEdit3("Window Base", sm.win_bg);
                ImGui::ColorEdit3("Input Frames", sm.frame_bg);
                ImGui::ColorEdit3("Title Base", sm.title_bg);
                ImGui::ColorEdit3("Active Title", sm.title_act);
                ImGui::ColorEdit3("Button Base", sm.btn_col);
                ImGui::ColorEdit3("Button Hovered", sm.btn_hov);
                ImGui::ColorEdit3("Header", sm.header_col);
                ImGui::ColorEdit3("Base Tab", sm.tab_col);
                ImGui::ColorEdit3("Active Tab", sm.tab_act);
                ImGui::ColorEdit3("Accent Color", sm.accent_col);

                ImGui::Spacing();
                if (ImGui::Button("Apply Palette Changes", ImVec2(180, 0))) {
                    sm.apply_current_styling();
                    sm.save_profile(current_profile);
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Geometry")) {
                ImGui::Spacing();
                ImGui::SliderFloat("Window Corners", &sm.window_rounding, 0.0f, 16.0f, "%.1f px");
                ImGui::SliderFloat("Frame Corners", &sm.frame_rounding, 0.0f, 12.0f, "%.1f px");
                ImGui::SliderFloat("Tab Corners", &sm.tab_rounding, 0.0f, 12.0f, "%.1f px");

                ImGui::Spacing();
                if (ImGui::Button("Apply Geometry", ImVec2(180, 0))) {
                    sm.apply_current_styling();
                    sm.save_profile(current_profile);
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
}
