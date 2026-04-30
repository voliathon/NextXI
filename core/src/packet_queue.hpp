#ifndef WINDOWER_PACKET_QUEUE_HPP
#define WINDOWER_PACKET_QUEUE_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace windower
{

enum class packet_direction
{
    incoming,
    outgoing,
};

class packet_queue
{
public:
    packet_queue(packet_queue const&) = delete;
    packet_queue(packet_queue&&)      = default;
    packet_queue(packet_direction direction) : m_direction{direction}
    {
        // Pre-allocate to prevent mid-combat heap reallocations
        m_output_buffer.reserve(4096);
    }

    void queue(
        std::uint16_t id, std::span<std::byte const> data,
        std::u8string_view injected_by);

    std::span<std::byte> temp_buffer(std::size_t);
    std::span<std::byte const> process_buffer(
        std::span<std::byte const>, std::uint16_t, std::uint32_t, std::size_t);

private:
    static constexpr std::size_t max_packet_size = 508;

    struct packet
    {
        packet(std::uint16_t, std::span<std::byte const>, std::u8string_view);

        std::uint16_t id;
        std::size_t size;
        std::array<std::byte, max_packet_size> data;
        std::u8string injected_by;
    };

    packet_direction const m_direction;
    std::deque<packet> m_queue;
    std::vector<std::byte> m_output_buffer;

    std::size_t peek_size() const noexcept;
    void process_packet(
        std::span<std::byte>&, std::uint16_t, std::uint16_t, std::uint32_t,
        std::span<std::byte const>, std::u8string_view = {}) const;
};

}

#endif