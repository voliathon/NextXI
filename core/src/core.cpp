#include "core.hpp"
#include "addon/error.hpp"
#include "addon/errors/package_error.hpp"
#include "command_handlers.hpp"
#include "command_manager.hpp"
#include "crash_handler.hpp"
#include "debug_console.hpp"
#include "hooks/advapi32.hpp"
#include "hooks/d3d8.hpp"
#include "hooks/ddraw.hpp"
#include "hooks/dinput8.hpp"
#include "hooks/ffximain.hpp"
#include "hooks/imm32.hpp"
#include "hooks/kernel32.hpp"
#include "hooks/user32.hpp"
#include "hooks/ws2_32.hpp"
#include "settings.hpp"
#include "ui/user_interface.hpp"
#include "ui/engine_console.hpp"
#include "unicode.hpp"
#include "utility.hpp"
#include "utilities/pol_hacks.hpp"
#include <float.h>
#include <gsl/gsl>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "utilities/logger.hpp"
#include <psapi.h>
#include <array>
#include <string_view>
#include <cstring>
#include "addon/modules/player_scanner.hpp"

namespace {
    uint32_t* g_pDivisorPtr = nullptr;
    bool g_scanner_finished = false;

    void enforce_fps_patch(int divisor) {
        if (g_scanner_finished) {
            if (g_pDivisorPtr != nullptr) {
                uint32_t current_val = 0;
                std::memcpy(&current_val, g_pDivisorPtr, sizeof(uint32_t));

                if (current_val != gsl::narrow_cast<uint32_t>(divisor)) {
                    DWORD oldProtect = 0;
                    ::VirtualProtect(g_pDivisorPtr, sizeof(uint32_t), PAGE_EXECUTE_READWRITE, &oldProtect);
                    uint32_t const new_val = gsl::narrow_cast<uint32_t>(divisor);
                    std::memcpy(g_pDivisorPtr, &new_val, sizeof(uint32_t));
                    ::VirtualProtect(g_pDivisorPtr, sizeof(uint32_t), oldProtect, &oldProtect);
                }
            }
            return;
        }

        HMODULE const hMod = ::GetModuleHandleW(L"FFXiMain.dll");
        if (hMod == nullptr) return;

        MODULEINFO modInfo{};
        if (::GetModuleInformation(::GetCurrentProcess(), hMod, &modInfo, sizeof(modInfo)) == 0) return;
        if (modInfo.lpBaseOfDll == nullptr) return;

        gsl::span<const uint8_t> const memory_span{
            static_cast<const uint8_t*>(modInfo.lpBaseOfDll),
            gsl::narrow_cast<size_t>(modInfo.SizeOfImage)
        };

        constexpr std::array<uint8_t, 12> pattern = { 0x81, 0xEC, 0x00, 0x01, 0x00, 0x00, 0x3B, 0xC1, 0x74, 0x21, 0x8B, 0x0D };
        constexpr std::string_view mask = "xxxxxxxxxxxx";

        for (size_t i = 0; i < memory_span.size() - mask.length(); ++i) {
            bool found = true;
            for (size_t j = 0; j < mask.length(); ++j) {
                if (gsl::at(mask, j) != '?' && gsl::at(pattern, j) != gsl::at(memory_span, i + j)) {
                    found = false;
                    break;
                }
            }

            if (found) {
                uint32_t global_var_addr = 0;
                auto const addr_span = memory_span.subspan(i + 12, sizeof(uint32_t));
                std::memcpy(&global_var_addr, addr_span.data(), sizeof(uint32_t));

                if (global_var_addr != 0) {
                    uint32_t struct_base_addr = 0;
                    auto const* const ptr_to_global = reinterpret_cast<const void*>(gsl::narrow_cast<uintptr_t>(global_var_addr));

                    if (ptr_to_global != nullptr) {
                        std::memcpy(&struct_base_addr, ptr_to_global, sizeof(uint32_t));

                        if (struct_base_addr != 0) {
                            uint32_t const final_addr = struct_base_addr + 0x30;
                            g_pDivisorPtr = reinterpret_cast<uint32_t*>(gsl::narrow_cast<uintptr_t>(final_addr));
                            windower::logger::sync_trace("core() -> FPS Signature found and locked!");
                        }
                    }
                }
                break;
            }
        }

        g_scanner_finished = true;
        if (g_pDivisorPtr == nullptr) {
            windower::logger::sync_trace("core() -> ERROR: Could not find FPS Signature!");
        }
    }
}

void windower::core::initialize() noexcept { instance(); }

windower::core& windower::core::instance() noexcept
{
    static core instance;
    return instance;
}

windower::core::core() noexcept
{
    windower::logger::sync_trace("core() -> Starting");

    incoming_packet_queue = std::make_unique<packet_queue>(packet_direction::incoming);
    outgoing_packet_queue = std::make_unique<packet_queue>(packet_direction::outgoing);
    addon_manager = std::make_unique<windower::addon_manager>();

    windower::logger::sync_trace("core() -> Installing kernel32");
    kernel32::install();

    windower::logger::sync_trace("core() -> Installing user32");
    user32::install();

    windower::logger::sync_trace("core() -> Installing advapi32");
    advapi32::install();

    windower::logger::sync_trace("core() -> Installing imm32");
    imm32::install();

    windower::logger::sync_trace("core() -> Installing d3d8");
    d3d8::install();

    windower::logger::sync_trace("core() -> Installing dinput8");
    dinput8::install();

    windower::logger::sync_trace("core() -> Installing ddraw");
    ddraw::install();

    windower::logger::sync_trace("core() -> Installing ws2_32");
    ws2_32::install();

    windower::logger::sync_trace("core() -> Loading settings");
    settings.load();

    windower::logger::sync_trace("core() -> Setting dump_path");
    crash_handler::instance().dump_path(settings.temp_path);

    windower::logger::sync_trace("core() -> Queueing next frame");
    run_on_next_frame([]() mutable {
        debug_console::initialize(core::instance().settings.debug);
        core::instance().package_manager =
            std::make_unique<windower::package_manager>();
        core::instance().package_manager->update_all();
        command_handlers::register_all();
        });

    windower::logger::sync_trace("core() -> Finished successfully");
}

void windower::core::output(
    std::u8string_view component, std::u8string_view text,
    command_source source)
{
    if (source < command_source::client)
    {
        auto u8_text = logger::process_output(component, text);
        u8_text.append(1, u8'\n');
        logger::queue_log(std::move(u8_text), false);
    }
    else
    {
        instance().run_on_next_frame([text = logger::process_output(component, text)] {
            ffximain::add_to_chat(text);
            });
    }
}

void windower::core::error(
    std::u8string_view component, std::u8string_view text,
    command_source source)
{
    std::ignore = source;
    auto u8_text = logger::process_output(component, text);
    u8_text.append(1, u8'\n');
    logger::queue_log(std::move(u8_text), true);
}

void windower::core::error(
    std::u8string_view component, std::exception const& exception,
    command_source source)
{
    error(component, logger::get_error_message(exception), source);
}

void windower::core::error(
    std::u8string_view component, std::exception_ptr exception,
    command_source source)
{
    if (exception)
    {
        try
        {
            std::rethrow_exception(exception);
        }
        catch (std::exception const& e)
        {
            error(component, e, source);
        }
    }
    else
    {
        error(component, u8"unknown error", source);
    }
}

void windower::core::update() noexcept
{
    if (!m_updated)
    {
        m_updated = true;

        // Update Window Title with Character Name
        static std::string s_current_character = "";
        bool const logged_in = windower::ffximain::is_logged_in();
        char const* const char_name = logged_in ? windower::player_scanner::get_cached_player_name() : nullptr;

        std::string const target_name = (char_name != nullptr) ? char_name : "";

        if (s_current_character != target_name)
        {
            s_current_character = target_name;
            std::string const new_title = target_name.empty() ? "NextXI" : "NextXI - " + target_name;

            struct enum_data {
                DWORD pid;
                HWND hwnd;
            } data = { ::GetCurrentProcessId(), nullptr };

            ::EnumWindows([](HWND h, LPARAM lParam) -> BOOL {
                auto* pData = reinterpret_cast<enum_data*>(lParam);
                DWORD pid = 0;
                ::GetWindowThreadProcessId(h, &pid);

                if (pid == pData->pid) {
                    std::array<char, 256> class_name{};
                    ::GetClassNameA(h, class_name.data(), gsl::narrow_cast<int>(class_name.size()));
                    if (std::string_view{ class_name.data() } == "FFXiClass") {
                        pData->hwnd = h;
                        return FALSE; // Stop enumeration once found
                    }
                }
                return TRUE;
                }, reinterpret_cast<LPARAM>(&data));

            if (data.hwnd != nullptr)
            {
                ::SetWindowTextA(data.hwnd, new_title.c_str());
            }
        }

        enforce_fps_patch(core::instance().settings.fps_divisor);

        class FpuStateGuard
        {
            unsigned int original_state;

        public:
            FpuStateGuard(FpuStateGuard const&) = delete;
            FpuStateGuard(FpuStateGuard&&) = delete;
            FpuStateGuard& operator=(FpuStateGuard const&) = delete;
            FpuStateGuard& operator=(FpuStateGuard&&) = delete;

            FpuStateGuard() noexcept
            {
                _controlfp_s(&original_state, 0, 0);
                _controlfp_s(nullptr, _PC_53, _MCW_PC);
            }

            ~FpuStateGuard()
            {
                _controlfp_s(nullptr, original_state, _MCW_PC);
            }
        };

        FpuStateGuard fpu_guard;

        windower::pol_hacks::apply();
        scheduler::next_frame();
        script_environment.run_until_idle();

        if (addon_manager)
        {
            addon_manager->run_until_idle();
        }

        std::queue<std::function<void()>> local_functions;
        {
            std::lock_guard<std::mutex> guard{ m_queued_functions_mutex };
            std::swap(m_queued_functions, local_functions);
        }

        while (!local_functions.empty())
        {
            try
            {
                local_functions.front()();
            }
            catch (std::exception const& e)
            {
                error(u8"Background Task", e, command_source::client);
            }
            local_functions.pop();
        }
    }
}

void windower::core::begin_frame() noexcept
{
    ui.begin_frame();
    m_updated = false;
}

void windower::core::end_frame() noexcept { ui.end_frame(); }

void windower::core::run_on_next_frame(std::function<void()> function)
{
    std::lock_guard<std::mutex> guard{ m_queued_functions_mutex };
    m_queued_functions.emplace(std::move(function));
}

std::optional<::LRESULT>
windower::core::process_message(::MSG const& msg) noexcept
{
    if (auto const result = ui.process_message(msg))
    {
        return result;
    }
    if (auto const result = binding_manager.process_message(msg))
    {
        return result;
    }
    return std::nullopt;
}
