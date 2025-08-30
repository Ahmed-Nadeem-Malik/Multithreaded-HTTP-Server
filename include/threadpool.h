/**
 * @file threadpool.h
 * @brief Thread pool implementation for concurrent client handling
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

/**
 * Thread pool for handling client connections concurrently
 */
class ThreadPool
{
   private:
    std::vector<std::thread> workers_;
    std::queue<int> client_queue_;           // Queue of client file descriptors
    std::mutex queue_mutex_;                 // Protects client queue
    std::condition_variable queue_cv_;       // Notifies workers of new tasks
    std::atomic<bool> stop_{false};          // Signal to stop all threads

    /**
     * Worker thread main loop that processes clients from queue
     */
    void worker();

   public:
    /**
     * Constructor that creates and starts worker threads
     * @param num_threads Number of worker threads to create
     */
    explicit ThreadPool(int num_threads);
    
    /**
     * Destructor that stops all threads and waits for completion
     */
    ~ThreadPool();

    /**
     * Submit client connection to thread pool for processing
     * @param client_fd Client socket file descriptor
     */
    void submit(int client_fd);
};
