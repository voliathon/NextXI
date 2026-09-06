#pragma once

#include <vector>
#include <string>

namespace windower::ui { class context; }

namespace windower::ui::tabs {

    struct addon_info {
        std::string name;
        bool is_loaded;
        bool has_readme;
        std::string readme_path;
    };

    void render_addon_table(windower::ui::context& ctx, const char* table_id, const std::vector<addon_info>& addons, bool is_player_loaded);
}
