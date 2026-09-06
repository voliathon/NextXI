#ifndef WINDOWER_SETTINGS_HPP
#define WINDOWER_SETTINGS_HPP

#include "enums.hpp"
#include "geometry.hpp"

#include <cstddef>
#include <filesystem>
#include <string>
#include <type_traits>
#include <vector>

namespace windower
{

class settings
{
public:
    bool debug          = false;
    bool developer_mode = false;

    std::u8string display_device_name;

    window_type window_type = window_type::window;

    rectangle display_bounds = rectangle{0, 0, 1280, 720};
    rectangle window_bounds  = rectangle{0, 0, 1280, 720};
    dimension render_size    = dimension{1280, 720};
    dimension ui_size        = dimension{1280, 720};

    bool hardware_mouse = true;

    unsigned int max_sounds        = 32;
    bool play_sound_when_unfocused = false;

    unsigned int mipmapping                 = 0;
    bool bump_mapping                       = false;
    bool map_compression                    = false;
    texture_compression texture_compression = texture_compression::uncompressed;
    environment_animation environment_animation = environment_animation::smooth;
    font_type font_type                         = font_type::uncompressed;

    float gamma = 2.2f;

    bool driver_stability = false;
    bool play_intro       = true;

    bool verbose_logging = true;

    bool pol_account_limit = false;
    bool pol_fast_login = false;
    bool pol_no_throttle = false;

    //Graphics Engine and VRAM Telemetry
    std::u8string graphics_engine;
    int vram_allocation = 1024;
    int fps_divisor = 2;

    std::filesystem::path settings_path;
    std::filesystem::path user_path;
    std::filesystem::path temp_path;

    std::u8string command_line_args;

    void load();
};

}

#endif
