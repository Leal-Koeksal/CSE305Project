#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t num_threads) : stop(false), active_tasks(0) {
    //std::cout << "[ThreadPool] Constructed with " << num_threads << " threads." << std::endl;
    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this, i]() {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    //debug
                    //std::cout << "[worker " << i << "] Waiting for task..." << std::endl;
                    condition.wait(lock, [this]() {
                        return stop || !tasks.empty();
                    });
                    if (stop && tasks.empty()) {
                        //debug
                        //std::cout << "[worker " << i << "] Exiting." << std::endl;
                        return; // Exit thread
                    }

                    task = std::move(tasks.front());
                    tasks.pop();
                    //debug
                    //std::cout << "[worker " << i << "] Dequeued task. Queue size: " << tasks.size() << std::endl;
                }

                //debug
                //std::cout << "[worker " << i << "] Starting task. Active tasks: " << active_tasks.load() << std::endl;

                try {
                    task();
                } catch (...) {
                    //debug
                    //std::cout << "[worker " << i << "] Task threw exception!" << std::endl;
                    // Handle exceptions inside task gracefully
                    active_tasks--;
                    {
                        std::lock_guard<std::mutex> lock(done_mutex);
                        cv_done.notify_all();
                    }
                    throw; // rethrow if you want to propagate or handle otherwise
                }
                //debug
                //std::cout << "[worker " << i << "] Finished task. Active tasks: " << active_tasks.load() - 1 << std::endl;

                // Task finished successfully
                active_tasks--;
                {
                    std::lock_guard<std::mutex> lock(done_mutex);
                    if (active_tasks == 0 && tasks.empty()) {
                        //debug
                        //std::cout << "[worker " << i << "] All tasks done. Notifying waiters." << std::endl;
                        cv_done.notify_all();
                    }
                }
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    //std::cout << "[ThreadPool] Destructor called." << std::endl;
    //std::cout << "[~ThreadPool] Stopping thread pool." << std::endl;
    stop = true;
    condition.notify_all();
    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();
            //std::cout << "[~ThreadPool] Joined one worker thread." << std::endl;
        }
    }
    //std::cout << "[~ThreadPool] All worker threads joined." << std::endl;
}

void ThreadPool::wait() {
    std::unique_lock<std::mutex> lock(done_mutex);
    //std::cout << "[wait] Waiting for all tasks to finish. Active tasks: " << active_tasks.load() << ", Queue size: " << tasks.size() << std::endl;
    cv_done.wait(lock, [this]() {
        bool done = (active_tasks == 0 && tasks.empty());
        //std::cout << "[wait] Checking done condition: " << done << std::endl;
        return done;
    });
    //std::cout << "[wait] All tasks finished." << std::endl;
}
