#include "hooks/ws2_32.hpp"

#include "core.hpp"
#include "hooklib/hook.hpp"
#include "utilities/pol_hacks.hpp"

#include <winsock2.h>
#include <string_view>
#include <algorithm>

namespace
{

namespace hooks
{
windower::hooklib::hook<decltype(::recv)> recv;
}

namespace callbacks
{
    constexpr auto pml_ip = 202 | 67 << 8 | 54 << 16 | 55 << 24;
    constexpr std::string_view pml_response{ "HTTP/1.1 200 OK\r\nContent-Length: 68\r\n\r\n<pml><body><timer href=\"gameto:1\" enable=\"1\" delay=\"0\"></body></pml>" };
    bool has_fast_login_executed = false;

    [[gsl::suppress("bounds.1"), gsl::suppress("type.1")]]
    bool check_fast_login(::SOCKET socket)
    {
        ::sockaddr peer;
        int peerlen = sizeof peer;
        if (::getpeername(socket, &peer, &peerlen) != 0)
        {
            return false;
        }

        auto const data = reinterpret_cast<std::uint8_t*>(peer.sa_data);
        auto const port = (data[0] << 8) | data[1];
        auto const ip = *reinterpret_cast<std::uint32_t*>(data + 2);
        return ip == pml_ip && port != 443;
    }

    int fast_login(char* buffer, int length, int flags)
    {
        std::ignore = flags; // Fixes the es.48 warning!
        size_t to_send = static_cast<size_t>(length) < pml_response.size() ? static_cast<size_t>(length) : pml_response.size();
        std::copy(pml_response.begin(), pml_response.begin() + to_send, buffer);
        return static_cast<int>(to_send);
    }

    int WSAAPI recv(::SOCKET s, char* buf, int len, int flags)
    {
        windower::pol_hacks::apply();
        static bool enable_fast_login = true;

        if (enable_fast_login && windower::core::instance().settings.pol_fast_login)
        {
            if (check_fast_login(s))
            {
                enable_fast_login = false;
                return fast_login(buf, len, flags);
            }
        }

        if (hooks::recv)
        {
            return hooks::recv(s, buf, len, flags);
        }
        return ::recv(s, buf, len, flags);
    }
}
}

void windower::ws2_32::install()
{
    if (!hooks::recv)
    {
        ::LoadLibraryW(L"ws2_32.dll"); // Force local proxy resolution

        hooks::recv = hooklib::make_hook(u8"ws2_32.dll", u8"recv", callbacks::recv);
    }
}

void windower::ws2_32::uninstall() noexcept
{
    hooks::recv = {};
}
