#include "debug_scanner.hpp"
#include "debug_scanner_value.hpp"
#include "debug_scanner_entity.hpp"
#include "addon/modules/player_scanner.hpp"
#include <imgui.h>
#include "addon/modules/fps_unlocker.hpp"

namespace windower::ui::debug_scanner
{
    void render_debug_tab(logger_callback const& log, bool& focus_console) noexcept {
        static bool s_show_memory_scanner = false;
        static bool s_show_entity_scanner = false;

        if (ImGui::BeginTabItem("Debug Tools", nullptr, 0)) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Diagnostic and execution tools for the NextXI engine.");
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

            // --- TOOL 1: MEMORY SCANNER ---
            if (ImGui::Button("Memory Scanner", ImVec2(150, 30))) {
                s_show_memory_scanner = true;
            }
            ImGui::SameLine();
            ImGui::TextWrapped("In-game 'Cheat Engine' to locate and manipulate dynamic 4-byte integers in FFXiMain.dll (e.g., Framerate Divisor).");

            ImGui::Spacing();

            // --- TOOL 2: ENTITY SCANNER ---
            if (ImGui::Button("Entity Scanner", ImVec2(150, 30))) {
                s_show_entity_scanner = true;
            }
            ImGui::SameLine();
            ImGui::TextWrapped("Extract global entity arrays and locate player status blocks in active memory.");

            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

            // --- TOOL 3: FRAMERATE UNLOCKER ---
            ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Engine Rendering & Framerate (WIP)");
            ImGui::Spacing();

            // Disabled until multi-level pointer map is built
            ImGui::BeginDisabled();

            if (ImGui::Button("Set 60 FPS", ImVec2(150, 30))) {
                windower::addon::modules::fps::set_fps_divisor(1, log);
            }
            ImGui::SameLine();
            ImGui::TextWrapped("Force engine to update at 60 frames per second (Divisor 1).");

            ImGui::Spacing();
            if (ImGui::Button("Set 30 FPS", ImVec2(150, 30))) {
                windower::addon::modules::fps::set_fps_divisor(2, log);
            }
            ImGui::SameLine();
            ImGui::TextWrapped("Restore default game speed (Divisor 2).");

            ImGui::EndDisabled();

            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
            // --- TOOL 4: AUTH RESET ---
            if (ImGui::Button("Reset Auth", ImVec2(150, 30))) {
                windower::player_scanner::reset_scan();
                log(u8"--- MANUAL AUTHENTICATION RESET ---");
            }
            ImGui::SameLine();
            ImGui::TextWrapped("Manually reset the authentication block scanner.");

            ImGui::EndTabItem();
        }

        // Dispatch render calls to the individual modules
        render_value_scanner_window(s_show_memory_scanner, log, focus_console);
        render_entity_scanner_window(s_show_entity_scanner, log, focus_console);
    }
}
