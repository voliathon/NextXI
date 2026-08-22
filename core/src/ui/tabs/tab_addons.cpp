#include "tab_addons.hpp"
#include "ui/addon_browser.hpp"
#include "addon/profile_manager.hpp"
#include "addon/modules/player_scanner.hpp"
#include "command_manager.hpp"
#include "core.hpp"
#include "ui/context.hpp"
#include <imgui.h>
#include <fstream>

namespace windower::ui::tabs {

    // Helper to read the file content into the ImGui popup
    static std::string load_file_content(const std::string& path) {
        std::ifstream ifs(path, std::ios::binary);
        if (ifs.is_open()) {
            return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        }
        return "Failed to load Readme.";
    }

    // Custom ImGui LED Status Indicator
    static void RenderStatusLED(bool is_active) {
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        float line_height = ImGui::GetTextLineHeight();
        float radius = line_height * 0.35f;

        // Center the dot vertically with the text
        ImVec2 center = ImVec2(p.x + radius + 4.0f, p.y + line_height * 0.5f + 2.0f);

        if (is_active) {
            // Draw an outer "glow" ring
            draw_list->AddCircleFilled(center, radius * 1.8f, IM_COL32(50, 255, 50, 60));
            // Draw the bright green core
            draw_list->AddCircleFilled(center, radius, IM_COL32(50, 220, 50, 255));

            // Shift the text over so it doesn't overlap the dot
            ImGui::SetCursorScreenPos(ImVec2(p.x + radius * 3.5f + 4.0f, p.y));
            ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "Active");
        }
        else {
            // Draw a flat grey core
            draw_list->AddCircleFilled(center, radius, IM_COL32(90, 90, 90, 255));

            ImGui::SetCursorScreenPos(ImVec2(p.x + radius * 3.5f + 4.0f, p.y));
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Idle");
        }
    }

    void render_addon_table(windower::ui::context& ctx, const char* table_id, const std::vector<addon_info>& addons, bool is_player_loaded) {

        if (ImGui::BeginTable(table_id, 4, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_PadOuterX)) {

            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 85.0f);
            ImGui::TableSetupColumn("Addon Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Auto-Load", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 140.0f); // Slimmed down since Settings is gone
            ImGui::TableHeadersRow();

            // Grab the active character profile name directly from the scanner
            char const* name_ptr = windower::player_scanner::get_cached_player_name();
            std::string current_profile = (name_ptr && name_ptr[0] != '\0') ? name_ptr : "global";

            for (const auto& addon : addons) {
                ImGui::TableNextRow();

                // ==========================================
                // Column 1: Live LED Status Badge
                // ==========================================
                ImGui::TableNextColumn();
                RenderStatusLED(addon.is_loaded);

                // ==========================================
                // Column 2: Addon Name
                // ==========================================
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(addon.name.c_str());

                // ==========================================
                // Column 3: Auto-Load Checkbox
                // ==========================================
                ImGui::TableNextColumn();

                // Fetch the REAL auto-load status from your physical .txt file
                bool auto_load_val = windower::ui::addon_browser::is_autoload_enabled(addon.name, current_profile);

                if (!is_player_loaded) {
                    ImGui::BeginDisabled();
                    ImGui::Checkbox((std::string("##al_") + addon.name).c_str(), &auto_load_val);
                    ImGui::EndDisabled();
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                        ImGui::SetTooltip("Login with a character to manage profiles.");
                    }
                }
                else {
                    if (ImGui::Checkbox((std::string("##al_") + addon.name).c_str(), &auto_load_val)) {
                        // Update the .txt file so the engine executes it on boot
                        windower::ui::addon_browser::toggle_autoload(addon.name, current_profile);

                        // Keep the profile manager synced (preserving whatever launch_hidden was previously set to under the hood)
                        auto boot_cfg = windower::profile_manager::instance().get_addon_config(reinterpret_cast<const char8_t*>(addon.name.c_str()));
                        windower::profile_manager::instance().set_addon_config(
                            reinterpret_cast<const char8_t*>(addon.name.c_str()),
                            auto_load_val,
                            boot_cfg.launch_hidden
                        );
                    }
                }

                // ==========================================
                // Column 4: Text Actions
                // ==========================================
                ImGui::TableNextColumn();

                if (addon.is_loaded) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
                    if (ImGui::Button((std::string("Unload##cmd_un_") + addon.name).c_str())) {
                        core::instance().run_on_next_frame([cmd = u8"/unload " + std::u8string(addon.name.begin(), addon.name.end())]() {
                            command_manager::instance().handle_command(cmd, command_source::console);
                            });
                    }
                    ImGui::PopStyleColor();
                }
                else {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
                    if (ImGui::Button((std::string("Load##cmd_ld_") + addon.name).c_str())) {
                        core::instance().run_on_next_frame([cmd = u8"/load " + std::u8string(addon.name.begin(), addon.name.end())]() {
                            command_manager::instance().handle_command(cmd, command_source::console);
                            });
                    }
                    ImGui::PopStyleColor();
                }

                if (addon.has_readme) {
                    ImGui::SameLine();
                    if (ImGui::Button((std::string("Info##rd_") + addon.name).c_str())) {
                        ImGui::OpenPopup((std::string("readme_popup_") + addon.name).c_str());
                    }
                }

                // ==========================================
                // Floating Context Menus
                // ==========================================
                if (ImGui::BeginPopupModal((std::string("readme_popup_") + addon.name).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("%s - README", addon.name.c_str());
                    ImGui::Separator();

                    ImGui::BeginChild("ReadmeScroll", ImVec2(550, 400), true);
                    ImGui::TextWrapped("%s", load_file_content(addon.readme_path).c_str());
                    ImGui::EndChild();

                    ImGui::Spacing();
                    if (ImGui::Button("Close", ImVec2(120, 0))) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            }
            ImGui::EndTable();
        }
    }
}
