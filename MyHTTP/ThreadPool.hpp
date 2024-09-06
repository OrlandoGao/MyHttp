//
//  ThreadPool.hpp
//  MyHTTP
//
//  Created by Orlando‘s Mac on 2024/9/6.
//

#ifndef ThreadPool_hpp
#define ThreadPool_hpp



// 线程池类
class ThreadPool {
public:
    // 构造函数：初始化线程池，启动指定数量的工作线程
    ThreadPool(size_t threads);
    
    // 销毁线程池
    ~ThreadPool();

    // 提交任务到线程池，返回一个 future 对象，用于获取任务的结果
    void enqueue(std::function<void()> func);

private:
    // 工作线程函数，不断从任务队列中取任务并执行
    void worker();

    // 线程池中的工作线程
    std::vector<std::thread> workers;
    
    // 任务队列，存放需要执行的任务
    std::queue<std::function<void()>> tasks;
    
    // 互斥锁，用于同步访问任务队列
    std::mutex queue_mutex;
    
    // 条件变量，用于通知线程有新任务到来
    std::condition_variable condition;
    
    // 控制线程池停止
    std::atomic<bool> stop;
};


#endif /* ThreadPool_hpp */
