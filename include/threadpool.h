#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

// Thread pool for handling client connections concurrently
class ThreadPool
{
   private:
    std::vector<std::thread> workers_;
    std::queue<int> client_queue_;           // Queue of client file descriptors
    std::mutex queue_mutex_;                 // Protects client queue
    std::condition_variable queue_cv_;       // Notifies workers of new tasks
    std::atomic<bool> stop_{false};          // Signal to stop all threads

    // Worker thread main loop
    void worker();

   public:
    explicit ThreadPool(int num_threads);
    ~ThreadPool();

    // Submit client connection to thread pool
    void submit(int client_fd);
};
