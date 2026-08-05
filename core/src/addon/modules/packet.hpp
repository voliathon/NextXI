#ifndef WINDOWER_ADDON_MODULES_PACKET_HPP
#define WINDOWER_ADDON_MODULES_PACKET_HPP

#include "addon/lua.hpp"
#include "addon/modules/event.hpp"

#include <cstdint>
#include <deque>
#include <span>

namespace windower
{

namespace detail
{

struct packet_result_data
{
    packet_result_data(std::uint16_t id, std::vector<std::byte> data) noexcept :
        id{id}, data{std::move(data)}
    {}

    std::uint16_t id;
    std::vector<std::byte> data;
};

}

class packet_result : public basic_result<detail::packet_result_data>
{
public:
    using basic_result::basic_result;

    std::uint16_t id() const noexcept;
    std::span<std::byte const> data() const noexcept;
};

packet_result trigger_packet(
    bool incoming, std::uint16_t id, std::uint16_t counter,
    std::uint32_t timestamp, std::span<std::byte const> data,
    std::u8string_view injected_by = {});

int load_packet_module(lua::state);

}

#endif