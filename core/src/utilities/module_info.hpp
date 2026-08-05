#ifndef WINDOWER_UTILITIES_MODULE_INFO_HPP
#define WINDOWER_UTILITIES_MODULE_INFO_HPP

namespace windower
{
    void* module_for(void const* ptr) noexcept;
    void* windower_module() noexcept;
    bool is_windower_module(void const* ptr) noexcept;
    bool is_game_module(void const* ptr) noexcept;
}

#endif
