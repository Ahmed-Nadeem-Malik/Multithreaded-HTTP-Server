#include "threadpool.hpp"

#include "server.hpp"

void ThreadPool::worker()
{
    while (true)
    {
        int client_fd;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] { return stop_.load() || !client_queue_.empty(); });

            if (stop_.load() && client_queue_.empty())
            {
                return;
            }

            client_fd = client_queue_.front();
            client_queue_.pop();
        }

        handle_client(client_fd);
    }
}

ThreadPool::ThreadPool(int num_threads)
{
    for (int i = 0; i < num_threads; ++i)
    {
        workers_.emplace_back([this] { worker(); });
    }
}

ThreadPool::~ThreadPool()
{
    stop_.store(true);
    queue_cv_.notify_all();

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
    queue_cv_.notify_one();
}
