#include "ThreadPool.h"

#include <type_traits>


ThreadPool::ThreadPool(size_t threadsNum)
    :stop_(false)
{
    for(size_t i = 0; i < threadsNum; i++)
    {
        workThreads_.emplace_back(std::thread(&ThreadPool::Worker, this));  
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        stop_ = true;
    }
    condition_.notify_all();
    for(auto& thread : workThreads_)
        thread.join();
}

void ThreadPool::Worker()
{
    while(true)
    {
        threadTask task;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            condition_.wait(lock, [this](){
                return stop_ || !taskQue_.empty();
            });

            if(stop_ && taskQue_.empty()) return;

            task = std::move(taskQue_.front());
            taskQue_.pop();
        }
        task();
    }
}


