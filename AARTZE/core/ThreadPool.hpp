// File: AARTZE/core/ThreadPool.hpp
#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

class ThreadPool
{
   public:
    explicit ThreadPool(size_t threads);

    template <class F>
    auto enqueue(F&& f) -> std::future<typename std::invoke_result_t<F>>;

    ~ThreadPool();

   private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
};

inline ThreadPool::ThreadPool(size_t threads) : stop(false)
{
    for (size_t i = 0; i < threads; ++i)
    {
        workers.emplace_back([this]() {
            for (;;)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    condition.wait(lock, [this]() { return stop || !tasks.empty(); });
                    if (stop && tasks.empty()) return;
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        });
    }
}

template <class F>
auto ThreadPool::enqueue(F&& f) -> std::future<typename std::invoke_result_t<F>>
{
    using result_t = typename std::invoke_result_t<F>;
    auto task =
        std::make_shared<std::packaged_task<result_t()>>(std::forward<F>(f));
    std::future<result_t> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
}

inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread& worker : workers)
    {
        if (worker.joinable()) worker.join();
    }
}
