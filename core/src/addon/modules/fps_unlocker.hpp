#pragma once
#include <cstdint>
#include <string_view>
#include <functional>

namespace windower::addon::modules::fps
{
    using logger_callback = std::function<void(std::u8string_view)>;

    // Attempts to find the FrameRateDivisor pointer via memory signature
    bool initialize_fps_pointer(logger_callback const& log) noexcept;

    // Sets the FPS divisor (1 = 60 FPS, 2 = 30 FPS, 0 = Uncapped)
    void set_fps_divisor(int32_t divisor, logger_callback const& log) noexcept;
}
