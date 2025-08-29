#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ThreadPool
{
   private:
    std::vector<std::thread> workers_;
    std::queue<int> client_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::atomic<bool> stop_{false};

    void worker();

   public:
    explicit ThreadPool(int num_threads);
    ~ThreadPool();
    
    void submit(int client_fd);
};