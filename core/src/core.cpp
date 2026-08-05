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

void windower::core::initialize() noexcept { instance(); }

windower::core& windower::core::instance() noexcept
{
    static core instance;
    return instance;
}
windower::core::core() noexcept
{
    // Allocate all core engine subsystems first!
    incoming_packet_queue = std::make_unique<packet_queue>(packet_direction::incoming);
    outgoing_packet_queue = std::make_unique<packet_queue>(packet_direction::outgoing);
    addon_manager = std::make_unique<windower::addon_manager>();

    // Install the FFXI hooks
    kernel32::install();
    user32::install();
    advapi32::install();
    imm32::install();
    d3d8::install();
    dinput8::install();
    ddraw::install();
    ws2_32::install();

    // Load configurations
    settings.load();
    crash_handler::instance().dump_path(settings.temp_path);

    // Defer the heavy initialization to the first frame
    run_on_next_frame([]() mutable {
        debug_console::initialize(core::instance().settings.debug);

        core::instance().package_manager =
            std::make_unique<windower::package_manager>();
        core::instance().package_manager->update_all();

        command_handlers::register_all();
        });
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
        } // Mutex is instantly unlocked here!
        while (!local_functions.empty())
        {
            try
            {
                local_functions.front()();
            }
            catch (std::exception const& e) // <--- Catch the variable 'e'
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
    std::lock_guard<std::mutex> guard{m_queued_functions_mutex};
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
