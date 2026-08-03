#ifndef WINDOWER_UTILITIES_PATHS_HPP
#define WINDOWER_UTILITIES_PATHS_HPP

#include <filesystem>

namespace windower
{
    std::filesystem::path windower_path();
    std::filesystem::path settings_path();
    std::filesystem::path user_path();
    std::filesystem::path temp_path();
    std::filesystem::path client_path();
}

#endif
