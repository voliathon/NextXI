#pragma once
#include <span>
#include <cstddef>
#include <cstdint>

namespace windower::network {
    void parse_incoming_packet(uint16_t id, std::span<std::byte const> data);
}
