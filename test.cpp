#include <iostream>
#include <thread>
#include <future>
#include "ThreadPool.h"
using namespace std;

mutex mut;
int func(int x) {
    auto now = time(nullptr);
    auto dateTime = localtime(&now);
    mut.lock(); // 为了防止打印错乱，我们在这里加锁
    cout << "任务编号：" << x <<" 执行线程ID: " << this_thread::get_id() << " 当前时间: " << dateTime->tm_min << ":" << dateTime->tm_sec << endl;
    mut.unlock();
    this_thread::sleep_for(std::chrono::milliseconds(2000));
    return x;
}


int main() {
    ThreadPool threadPool;
    for (int i = 0; i < 20; ++i) auto ret = threadPool.AddTask(func, i);
    return 0;
}