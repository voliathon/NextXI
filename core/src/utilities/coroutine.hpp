#pragma once

#include <coroutine>
#include <future>
#include <type_traits>

template<typename R, typename T, typename... Args>
    requires(!std::is_void_v<R> && !std::is_reference_v<R>)
struct std::coroutine_traits<std::future<R>, T&, Args...>
{
    struct promise_type : std::promise<R>
    {
        std::future<R> get_return_object() noexcept
        {
            return this->get_future();
        }

        std::suspend_never initial_suspend() const noexcept { return {}; }
        std::suspend_never final_suspend() const noexcept { return {}; }

        void return_value(R const& value) noexcept(
            std::is_nothrow_copy_constructible_v<R>)
        {
            this->set_value(value);
        }

        void return_value(R&& value) noexcept(
            std::is_nothrow_move_constructible_v<R>)
        {
            this->set_value(std::move(value));
        }

        void unhandled_exception() noexcept
        {
            this->set_exception(std::current_exception());
        }
    };
};

template<typename T, typename... Args>
struct std::coroutine_traits<std::future<void>, T&, Args...>
{
    struct promise_type : std::promise<void>
    {
        std::future<void> get_return_object() noexcept
        {
            return this->get_future();
        }

        std::suspend_never initial_suspend() const noexcept { return {}; }
        std::suspend_never final_suspend() const noexcept { return {}; }

        void return_void() noexcept { this->set_value(); }

        void unhandled_exception() noexcept
        {
            this->set_exception(std::current_exception());
        }
    };
};
template<typename R, typename T, typename... Args>
    requires(!std::is_void_v<R> && !std::is_reference_v<R>)
struct std::coroutine_traits<std::future<R>, T*, Args...> :
    std::coroutine_traits<std::future<R>, T&, Args...>
{};

template<typename T, typename... Args>
struct std::coroutine_traits<std::future<void>, T*, Args...> :
    std::coroutine_traits<std::future<void>, T&, Args...>
{};

template<typename R>
auto operator co_await(std::future<R> future) noexcept
    requires(!std::is_reference_v<R>)
{
    struct awaiter : std::future<R>
    {
        bool await_ready() const noexcept
        {
            return this->wait_for(std::chrono::seconds(0)) !=
                   std::future_status::timeout;
        }

        void await_suspend(std::coroutine_handle<> continuation) const
        {
            std::thread([this, continuation] {
                this->await_ready();
                continuation();
            }).detach();
        }

        R await_resume() { return this->get(); }
    };

    return awaiter{std::move(future)};
}