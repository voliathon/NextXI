#ifndef WINDOWER_ADDON_SCRIPT_BASE_HPP
#define WINDOWER_ADDON_SCRIPT_BASE_HPP

#include "addon/lua.hpp"
#include "addon/lua_internal.hpp"
#include "addon/package_manager.hpp"
#include "addon/scheduler.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <tuple>

namespace windower
{

class script_base
{
public:
    script_base(script_base const&) = delete;
    script_base(script_base&&)      = delete;

    script_base& operator=(script_base const&) = delete;
    script_base& operator=(script_base&&)      = delete;

    static script_base* get_script_base(lua::state);

    wait_state run_until_idle();
    std::weak_ptr<lua::state> root_handle() const noexcept;

    void schedule(windower::task&&);

    template<typename F, typename... A>
    void schedule(F const& function, A&&... args)
    {
        m_scheduler.schedule(
            function(lua::state{m_interpreter}, std::forward<A>(args)...));
    }

    virtual std::shared_ptr<windower::package const>
        find_dependency(lua::state, std::u8string_view) const noexcept(false);

protected:
    lua::interpreter m_interpreter;
    std::shared_ptr<lua::state> m_root_handle;
    scheduler m_scheduler;

    script_base() noexcept;

    ~script_base() = default;

    void reset();
};

namespace lua
{

class coroutine_handle : public state
{
public:
    coroutine_handle(state);
    coroutine_handle(coroutine_handle const&)     = delete;
    coroutine_handle(coroutine_handle&&) noexcept = default;

    ~coroutine_handle();

    coroutine_handle& operator=(coroutine_handle const&)     = delete;
    coroutine_handle& operator=(coroutine_handle&&) noexcept = default;
};

std::tuple<bool, windower::wait_state>
schedulable_resume(stack_guard&, std::size_t);

}

}

#endif