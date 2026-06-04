/*
 * Copyright © Windower Dev Team
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"),to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
constexpr std::string_view pml_response{"HTTP/1.1 200 OK\r\nContent-Length: 68\r\n\r\n<pml><body><timer href=\"gameto:1\" enable=\"1\" delay=\"0\"></body></pml>"};
bool has_fast_login_executed = false;

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
    // PlayOnline Viewer connects to 202.67.54.55 on port 443 for TLS and port 80 for PML.
    // We MUST ignore port 443 (TLS) otherwise returning plaintext HTTP causes a fatal crash.
    return ip == pml_ip && port != 443;
}

int fast_login(char* buffer, int length, int flags)
{
    (void)flags;
    // Copy the payload up to the requested length.
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
        hooks::recv = hooklib::make_hook(u8"ws2_32.dll", u8"recv", callbacks::recv);
    }
}

void windower::ws2_32::uninstall() noexcept
{
    hooks::recv = {};
}
