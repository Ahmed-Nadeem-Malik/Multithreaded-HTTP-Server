#include "../include/threadpool.h"

#include "../include/server.h"

void ThreadPool::worker()
{
    while (true)
    {
        int client_fd;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            // Wait for work or stop signal
            queue_cv_.wait(lock, [this] { return stop_.load() || !client_queue_.empty(); });

            // Exit if stopping and no work remains
            if (stop_.load() && client_queue_.empty())
            {
                return;
            }

            // Get next client from queue
            client_fd = client_queue_.front();
            client_queue_.pop();
        }

        // Process client request
        handle_client(client_fd);
    }
}

ThreadPool::ThreadPool(int num_threads)
{
    // Start worker threads
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

    // Wait for all threads to finish
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
        std::lock_guard<std::mutex> lock(queue_mutex_);
        client_queue_.push(client_fd);
    }
    // Wake up one waiting worker
    queue_cv_.notify_one();
}
