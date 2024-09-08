//
//  ThreadPool.cpp
//  MyHTTP
//
//  Created by Orlando‘s Mac on 2024/9/6.
//

#include "ThreadPool.hpp"



// 构造函数：初始化线程池并启动指定数量的工作线程
ThreadPool::ThreadPool(size_t threads) {
    for (size_t i = 0; i < threads; ++i) {
        workers.emplace_back([this] { worker(); });
    }
}


// 析构函数：销毁线程池
ThreadPool::~ThreadPool() {
    //发出停止信号
    stop.store(true);
    condition.notify_all();
    //等待线程停止
    for (std::thread &worker : workers) {
        worker.join();
    }
}

// 提交任务到线程池
void ThreadPool::enqueue(std::function<void()> task) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    if (stop.load()) {
        throw std::runtime_error("enqueue on stopped ThreadPool");
    }
    tasks.emplace(task);
    
    condition.notify_one();
    return;
}

// 工作线程函数：不断从任务队列中取任务并执行
void ThreadPool::worker() {
    while (!stop.load()) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [this] { return stop.load() || !tasks.empty(); });

            if (stop.load() && tasks.empty()) {
                return;
            }

            task = std::move(tasks.front());
            tasks.pop();
        }

        task();
    }
}
