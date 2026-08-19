#include "ui/addon_browser.hpp"
#include "ui/engine_console.hpp"
#include "command_manager.hpp"
#include "core.hpp"
#include "addon/addon_manager.hpp"
#include "addon/modules/player_scanner.hpp" // Added to grab the character name
#include <imgui.h>
#include <fstream>

namespace windower::ui
{
    std::filesystem::path addon_browser::get_autoload_file(std::string const& profile)
    {
        std::error_code ec;
        auto dir = core::instance().settings.user_path.parent_path() / u8"addons";
        if (!std::filesystem::exists(dir, ec)) dir = std::filesystem::current_path() / u8"addons";
        return dir / ("autoload_" + profile + ".txt");
    }

    bool addon_browser::is_autoload_enabled(std::string const& addon_name, std::string const& profile)
    {
        std::ifstream file(get_autoload_file(profile));
        std::string line;
        while (std::getline(file, line)) if (line == addon_name) return true;
        return false;
    }

    void addon_browser::toggle_autoload(std::string const& addon_name, std::string const& profile)
    {
        auto path = get_autoload_file(profile);
        std::vector<std::string> addons;
        bool found = false;

        std::ifstream in(path);
        std::string line;
        while (std::getline(in, line))
        {
            if (line == addon_name) found = true;
            else if (!line.empty()) addons.push_back(line);
        }
        in.close();

        if (!found) addons.push_back(addon_name);

        std::ofstream out(path);
        for (auto const& a : addons) out << a << "\n";
    }

    void addon_browser::run_autoload(std::string const& profile)
    {
        auto path = get_autoload_file(profile);
        std::ifstream in(path);
        std::string line;
        while (std::getline(in, line))
        {
            if (!line.empty()) {
                core::instance().run_on_next_frame([cmd = u8"/load " + std::u8string(line.begin(), line.end())]() {
                    command_manager::instance().handle_command(cmd, command_source::console);
                    });
            }
        }
    }

    void addon_browser::check_directory_changes()
    {
        std::error_code ec;
        auto base_dir = core::instance().settings.user_path.parent_path() / u8"addons";
        if (!std::filesystem::exists(base_dir, ec)) base_dir = std::filesystem::current_path() / u8"addons";

        auto dir_nx = base_dir / u8"nextxi";
        auto dir_w4 = base_dir / u8"windower";

        auto time_nx = std::filesystem::exists(dir_nx, ec) ? std::filesystem::last_write_time(dir_nx, ec) : std::filesystem::file_time_type{};
        auto time_w4 = std::filesystem::exists(dir_w4, ec) ? std::filesystem::last_write_time(dir_w4, ec) : std::filesystem::file_time_type{};

        if (!ec && (time_nx != m_last_dir_time_nx || time_w4 != m_last_dir_time_w4))
        {
            m_last_dir_time_nx = time_nx;
            m_last_dir_time_w4 = time_w4;
            m_scanned = false;

            if (core::instance().package_manager)
            {
                core::instance().package_manager->reset();
            }
        }

        if (!m_scanned)
        {
            scan_addons();
            // The global autoload execution block was permanently removed from here.
            // Our session_tracker now handles this safely!
        }
    }

    void addon_browser::reset_scan() noexcept { m_scanned = false; }

    std::vector<addon_list_item> const& addon_browser::get_cached_addons() const noexcept { return m_cached_addons; }

    void addon_browser::scan_addons()
    {
        m_cached_addons.clear();
        std::error_code ec;
        auto base_dir = core::instance().settings.user_path.parent_path() / u8"addons";
        if (!std::filesystem::exists(base_dir, ec)) base_dir = std::filesystem::current_path() / u8"addons";

        auto scan_dir = [&](std::filesystem::path const& dir) {
            if (std::filesystem::exists(dir, ec))
            {
                for (auto const& entry : std::filesystem::directory_iterator{ dir, ec })
                {
                    if (entry.is_directory(ec))
                    {
                        addon_list_item item;
                        item.name = entry.path().filename().u8string();
                        auto const path = entry.path();

                        item.is_modern = std::filesystem::exists(path / u8"manifest.xml", ec);

                        std::filesystem::path const r_md = path / u8"README.md";
                        std::filesystem::path const r_low = path / u8"readme.md";
                        std::filesystem::path const r_txt = path / u8"readme.txt";

                        if (std::filesystem::exists(r_md, ec)) { item.has_readme = true; item.readme_path = r_md; }
                        else if (std::filesystem::exists(r_low, ec)) { item.has_readme = true; item.readme_path = r_low; }
                        else if (std::filesystem::exists(r_txt, ec)) { item.has_readme = true; item.readme_path = r_txt; }

                        if (item.is_modern || std::filesystem::exists(path / (item.name + u8".lua"), ec))
                        {
                            m_cached_addons.push_back(std::move(item));
                        }
                    }
                }
            }
            };

        scan_dir(base_dir / u8"nextxi");
        scan_dir(base_dir / u8"windower");

        m_scanned = true;
    }

    void addon_browser::load_readme(std::filesystem::path const& path)
    {
        std::ifstream ifs(path, std::ios::binary);
        if (ifs.is_open())
        {
            m_readme_content = std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            m_readme_title = "README: " + path.parent_path().filename().string();
            m_show_readme = true;
        }
    }

    void addon_browser::render_addon_list(bool is_modern)
    {
        if (is_modern) ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.3f, 1.0f), "NextXI Native Addons (Manifest Validated)");
        else ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Legacy Addons (No Manifest)");

        ImGui::Separator();

        // Dynamically pull the currently authenticated character profile
        char const* name_ptr = windower::player_scanner::get_cached_player_name();
        std::string current_profile = (name_ptr && name_ptr[0] != '\0') ? name_ptr : "global";

        if (ImGui::BeginChild(is_modern ? "ModernScroll" : "LegacyScroll"))
        {
            for (auto const& addon : m_cached_addons)
            {
                if (addon.is_modern != is_modern) continue;

                std::string const name_str(addon.name.begin(), addon.name.end());
                bool is_active = false;
                if (core::instance().addon_manager) {
                    is_active = (core::instance().addon_manager->get(addon.name) != nullptr);
                }

                // Explicitly pass the active profile string so we no longer read/write to the global file!
                bool const is_auto = is_autoload_enabled(name_str, current_profile);

                ImGui::Text("%-25s", name_str.c_str());

                ImGui::SameLine(180.0f);
                if (is_active) ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "[ ON  ]");
                else ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "[ OFF ]");

                ImGui::SameLine(250.0f);

                if (ImGui::Button(("Load##" + name_str).c_str()))
                {
                    core::instance().run_on_next_frame([cmd = u8"/load " + addon.name]() {
                        command_manager::instance().handle_command(cmd, command_source::console);
                        });
                }
                ImGui::SameLine();
                if (ImGui::Button(("Unload##" + name_str).c_str()))
                {
                    core::instance().run_on_next_frame([cmd = u8"/unload " + addon.name]() {
                        command_manager::instance().handle_command(cmd, command_source::console);
                        });
                }

                ImGui::SameLine();
                if (is_auto) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));

                // Explicitly pass the active profile string so the toggle writes to your character file
                if (ImGui::Button(("Auto-Load##" + name_str).c_str())) toggle_autoload(name_str, current_profile);

                if (is_auto) ImGui::PopStyleColor();

                if (addon.has_readme)
                {
                    ImGui::SameLine();
                    if (ImGui::Button(("Readme##" + name_str).c_str())) load_readme(addon.readme_path);
                }
            }
        }
        ImGui::EndChild();
    }

    void addon_browser::render_tabs()
    {
        if (ImGui::BeginTabItem("Windower 4 Addons"))
        {
            render_addon_list(false);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("NextXI Addons"))
        {
            render_addon_list(true);
            ImGui::EndTabItem();
        }
    }

    void addon_browser::render_readme_window()
    {
        if (m_show_readme)
        {
            ImGui::SetNextWindowSize(ImVec2(600, 450), ImGuiCond_FirstUseEver);
            if (ImGui::Begin(m_readme_title.c_str(), &m_show_readme))
            {
                ImGui::TextWrapped("%s", m_readme_content.c_str());
            }
            ImGui::End();
        }
    }
}
