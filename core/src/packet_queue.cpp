#include "packet_queue.hpp"
#include "addon/modules/packet.hpp"

#include <cstring>

namespace
{

    struct packet_header
    {
    public:
        packet_header() noexcept = default;
        packet_header(
            std::uint16_t id, std::size_t size, std::uint16_t counter) noexcept :
            m_packed_id_size{ gsl::narrow_cast<std::uint16_t>((id & 0x1FF) | (((size + sizeof(packet_header)) << 7) & 0xFE00)) },
            m_counter{ counter }
        {
        }

        std::size_t size() const noexcept
        {
            return (m_packed_id_size >> 7 & 0x1FC) - sizeof(packet_header);
        }
        std::uint16_t id() const noexcept { return m_packed_id_size & 0x1FF; }
        std::uint16_t counter() const noexcept { return m_counter; }

    private:
        std::uint16_t m_packed_id_size;
        std::uint16_t m_counter;
    };

    static_assert(sizeof(packet_header) == 4);


    // Use memcpy instead of std::copy_n to guarantee zero analyzer warnings and maximum speed
    template<typename T>
    T read(std::span<std::byte const>& input)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        T value;
        std::memcpy(&value, input.data(), sizeof(T));
        input = input.subspan(sizeof(T));
        return value;
    }

    std::span<std::byte const>
        read(std::span<std::byte const>& input, std::size_t size) noexcept
    {
        auto const value = input.subspan(0, size);
        input = input.subspan(size);
        return value;
    }

    std::span<std::byte const>
        write(std::span<std::byte>& output, std::span<std::byte const> value)
    {
        std::memcpy(output.data(), value.data(), value.size());
        output = output.subspan(value.size());
        return value;
    }

    template<typename T>
    T const& write(std::span<std::byte>& output, T const& value)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        std::memcpy(output.data(), &value, sizeof(T));
        output = output.subspan(sizeof(T));
        return value;
    }

}

void windower::packet_queue::queue(
    std::uint16_t id, std::span<std::byte const> data,
    std::u8string_view injected_by)
{
    std::lock_guard<std::mutex> lock{ m_queue_mutex };
    m_queue.emplace_back(id, data, injected_by);
}

std::span<std::byte const> windower::packet_queue::process_buffer(
    std::span<std::byte const> input, std::uint16_t counter,
    std::uint32_t timestamp, std::size_t output_size)
{
    if (m_output_buffer.size() < output_size)
    {
        m_output_buffer.clear();
        m_output_buffer.resize(output_size);
    }
    auto output = std::span{ m_output_buffer };

    while (input.size() >= sizeof(packet_header))
    {
        auto const header = read<packet_header>(input);
        auto const size = header.size();
        if (size > input.size())
        {
            continue;
        }
        auto const id = header.id();
        auto const data = read(input, size);
        if (data.size() + sizeof header <= output.size())
        {
            process_packet(output, id, counter, timestamp, data);
        }
        else
        {
            std::lock_guard<std::mutex> lock{ m_queue_mutex };
            m_queue.emplace_front(id, data, u8"");
            core::error(
                u8"",
                u8"WARNING!!! Client packet was delayed due to buffer "
                u8"overflow.");
        }
    }

    // Safely steal the queue contents to process without holding the lock
    std::deque<packet> local_queue;
    {
        std::lock_guard<std::mutex> lock{ m_queue_mutex };
        std::swap(m_queue, local_queue);
    }

    while (!local_queue.empty() && (local_queue.front().size + sizeof(packet_header)) <= output.size())
    {
        auto const& packet = local_queue.front();
        process_packet(
            output, packet.id, counter, timestamp,
            std::span{ packet.data.data(), packet.size }, packet.injected_by);
        local_queue.pop_front();
    }

    // If we couldn't process everything due to output limits, push it back safely
    if (!local_queue.empty())
    {
        std::lock_guard<std::mutex> lock{ m_queue_mutex };
        for (auto it = local_queue.rbegin(); it != local_queue.rend(); ++it)
        {
            m_queue.push_front(*it);
        }
    }

    return { m_output_buffer.data(), m_output_buffer.size() - output.size() };
}

std::size_t windower::packet_queue::peek_size() const noexcept
{
    return m_queue.front().size + sizeof(packet_header);
}

void windower::packet_queue::process_packet(
    std::span<std::byte>& output, std::uint16_t id, std::uint16_t counter,
    std::uint32_t timestamp, std::span<std::byte const> data,
    std::u8string_view injected_by) const
{
    using namespace windower;

    auto const result = trigger_packet(
        m_direction == packet_direction::incoming, id, counter, timestamp, data,
        injected_by);

    if (result.blocked())
    {
        return;
    }

    if (result.unchanged())
    {
        write(output, packet_header{id, data.size(), counter});
        write(output, data);
        return;
    }

    auto const result_id = result.id();
    auto const result_data = result.data();

    if (result_data.size() + sizeof(packet_header) > output.size())
    {
        core::error(
            u8"", u8"WARNING!!! Oversized packet modifications were dropped.");
        write(output, packet_header{id, data.size(), counter});
        write(output, data);
        return;
    }

    write(output, packet_header{result_id, result_data.size(), counter});
    write(output, result_data);
}

windower::packet_queue::packet::packet(
    std::uint16_t id, std::span<std::byte const> data_in,
    std::u8string_view injected_by) :
    id{id}, size{data_in.size()}, injected_by{injected_by}
{
    auto const copy_size = size > max_packet_size ? max_packet_size : size;
    std::copy_n(data_in.begin(), copy_size, data.begin());
}
