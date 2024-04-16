#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <future>


class ThreadPool
{
public:
    explicit ThreadPool(size_t threadsNum = std::thread::hardware_concurrency());
    ~ThreadPool();

    void Worker();

    template<typename F, typename... Args>
    auto AddTask(F&& f, Args&&... args) -> std::future<decltype(f(args...))>;
private:
    using threadTask = std::function<void()>;
    std::vector<std::thread> workThreads_;
    std::queue<threadTask> taskQue_;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    bool stop_;
};
// 可变参数模板，模板必须在头文件定义
template<typename F, typename... Args>
auto ThreadPool::AddTask(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
{
    // using returnType = std::invoke_result_t<F, Args...>; //only for c++17
    using returnType = decltype(f(args...));
    auto task = std::make_shared<std::packaged_task<returnType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<returnType> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        if(stop_) { throw std::runtime_error("enqueue on stopped Thread pool"); }

        // taskQue_.emplace([task = std::move(task)]() { (*task)(); }); // lambda capture initializers only available with ‘-std=c++14’ or ‘-std=gnu++14’
        taskQue_.emplace([task]() { (*task)(); });
    }
    condition_.notify_one();
    return res;
}