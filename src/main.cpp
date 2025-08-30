/**
 * MultithreadedServer - High-performance HTTP server with thread pool
 * Author: Ahmed Nadeem Malik
 */

#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "../include/config.h"
#include "../include/routing.h"
#include "../include/server.h"
#include "../include/threadpool.h"

int main()
{
    try
    {
        // Initialize HTTP routes
        init_routes();

        // Create and bind server socket
        int server_fd = setup_server_socket();
        std::cout << "Server listening on port " << Config::PORT << std::endl;

        // Create thread pool for handling clients
        ThreadPool pool(Config::THREADS_NUM);

        // Main server loop - accept and dispatch clients
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

            // Submit client to thread pool for processing
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
