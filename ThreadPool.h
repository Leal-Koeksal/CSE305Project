#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <iostream>

class ThreadPool {
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();

    template<class F>
    void enqueue(F&& f);

    void wait();

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;

    std::mutex done_mutex;
    std::condition_variable cv_done;

    std::atomic<bool> stop;
    std::atomic<int> active_tasks;
};

template<class F>
void ThreadPool::enqueue(F&& f) {
    active_tasks++; // Increment active_tasks here, before adding to queue
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.emplace(std::forward<F>(f));
        // debug
        // std::cout << "[enqueue] Task added. Queue size: " << tasks.size() << ", active_tasks: " << active_tasks.load() << std::endl;
    }
    condition.notify_one();
}
