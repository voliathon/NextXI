#ifndef WINDOWER_DOWNLOADER_HPP
#define WINDOWER_DOWNLOADER_HPP

#include <atomic>
#include <chrono>
#include <coroutine>
#include <cstdint>
#include <filesystem>
#include <future>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>

namespace windower
{

class downloader
{
public:
    enum class status
    {
        pending,
        downloading,
        error,
        complete,
        skipped,
    };

    struct file
    {
        std::u8string url;
        std::filesystem::path path;
        std::optional<std::chrono::system_clock::time_point> last_modified;
    };

    class result
    {
    public:
        downloader::file const& file() const noexcept;

        downloader::status status() const noexcept;

        std::uint16_t http_status() const noexcept;

        bool is_http_error() const noexcept;

        std::error_code error_code() const noexcept;

        explicit operator bool() const noexcept;

    private:
        struct impl;

        std::shared_ptr<impl> m_impl;

        result(downloader::file const&);

        friend class downloader;
        friend class job;
    };

    class job
    {
    public:
        std::future<std::vector<result>> future() const;

        auto operator co_await() const noexcept { return awaiter{*this}; }

    private:
        struct impl;

        class awaiter
        {
        public:
            awaiter(job const&) noexcept;

            bool await_ready() const noexcept;

            void await_suspend(std::coroutine_handle<>) const;

            std::vector<result> await_resume() const;

        private:
            job const& m_job;
        };

        std::shared_ptr<impl> m_impl;

        job(std::vector<result>&&);

        friend class downloader;
    };

    downloader() noexcept;
    downloader(downloader const&) = delete;
    downloader(downloader&&)      = delete;

    ~downloader();

    downloader& operator=(downloader const&) = delete;
    downloader& operator=(downloader&&)      = delete;

    job download(std::vector<file> const&);
    job download(file);
    job download(std::u8string, std::u8string);

private:
    std::atomic<bool> m_running;
    std::mutex m_mutex;
    std::queue<job> m_jobs;
    std::condition_variable m_job_added;
    std::thread m_thread;

    std::optional<job> get_next_job();

    void run();
};

}

#endif