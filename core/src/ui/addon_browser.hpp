#ifndef WINDOWER_UI_ADDON_BROWSER_HPP
#define WINDOWER_UI_ADDON_BROWSER_HPP

#include <string>
#include <vector>
#include <filesystem>

namespace windower::ui
{
    struct addon_list_item
    {
        std::u8string name;
        bool is_modern = false;
        bool has_readme = false;
        std::filesystem::path readme_path;
    };

    class addon_browser
    {
    public:
        void check_directory_changes();
        void reset_scan() noexcept;

        void render_tabs();
        void render_readme_window();

        std::vector<addon_list_item> const& get_cached_addons() const noexcept;
        static void run_autoload(std::string const& profile);

    private:
        bool m_scanned = false;

        // Monitor both folders independently!
        std::filesystem::file_time_type m_last_dir_time_nx;
        std::filesystem::file_time_type m_last_dir_time_w4;

        std::vector<addon_list_item> m_cached_addons;

        bool m_show_readme = false;
        std::string m_readme_title;
        std::string m_readme_content;

        void scan_addons();
        void load_readme(std::filesystem::path const& path);
        void render_addon_list(bool is_modern); // Caveman DRY code trick!

        static std::filesystem::path get_autoload_file(std::string const& profile);
        static bool is_autoload_enabled(std::string const& addon_name, std::string const& profile = "global");
        static void toggle_autoload(std::string const& addon_name, std::string const& profile = "global");
    };
}

#endif
