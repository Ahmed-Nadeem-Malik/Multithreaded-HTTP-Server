/**
 * MultithreadedServer - High-performance HTTP server with thread pool
 * Author: Ahmed Nadeem Malik
 */

#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>

#include "../include/config.h"
#include "../include/globals.h"
#include "../include/routing.h"
#include "../include/server.h"
#include "../include/threadpool.h"

int main()
{
    try
    {
        init_routes();

        int server_fd = setup_server_socket();
        std::cout << "Server listening on port " << Config::PORT << std::endl;

        ThreadPool pool(Config::THREADS_NUM);

        while (true)
        {
            struct sockaddr_storage client_addr;
            socklen_t addr_size = sizeof(client_addr);

            int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_size);
            if (client_fd == -1)
            {
                std::cerr << "Accept failed, continuing..." << std::endl;
                continue;
            }

            pool.submit(client_fd);
        }

        close(server_fd);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
