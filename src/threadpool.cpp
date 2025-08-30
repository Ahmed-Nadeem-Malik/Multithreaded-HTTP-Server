/**
 * @file threadpool.cpp
 * @brief ThreadPool implementation for concurrent client handling
 */

#include "../include/threadpool.h"

#include "../include/server.h"

void ThreadPool::worker()
{
    while (true)
    {
        int client_fd;
        {
            // Wait for either a client to process or shutdown signal
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] { return stop_.load() || !client_queue_.empty(); });

            // Exit if shutting down and no more clients to process
            if (stop_.load() && client_queue_.empty())
            {
                return;
            }

            // Get next client from queue
            client_fd = client_queue_.front();
            client_queue_.pop();
        }

        handle_client(client_fd);
    }
}

ThreadPool::ThreadPool(int num_threads)
{
    // Create and start worker threads
    for (int i = 0; i < num_threads; ++i)
    {
        workers_.emplace_back([this] { worker(); });
    }
}

ThreadPool::~ThreadPool()
{
    // Signal all threads to stop
    stop_.store(true);
    queue_cv_.notify_all();

    // Wait for all worker threads to complete
    for (auto& worker : workers_)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

void ThreadPool::submit(int client_fd)
{
    {
        // Add client to processing queue
        std::lock_guard<std::mutex> lock(queue_mutex_);
        client_queue_.push(client_fd);
    }
    // Wake up one worker thread to handle the new client
    queue_cv_.notify_one();
}
