#include "server.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <mutex>
#include <stdexcept>

#include "config.h"
#include "http.h"
#include "globals.h"

int setup_server_socket()
{
    struct addrinfo hints, *server_info;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(NULL, Config::PORT, &hints, &server_info);
    if (status != 0)
    {
        throw std::runtime_error("getaddrinfo failed: " + std::string(gai_strerror(status)));
    }

    int server_fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (server_fd == -1)
    {
        freeaddrinfo(server_info);
        throw std::runtime_error("Socket creation failed");
    }

    if (bind(server_fd, server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        close(server_fd);
        freeaddrinfo(server_info);
        throw std::runtime_error("Bind failed");
    }

    if (listen(server_fd, Config::BACKLOG) == -1)
    {
        close(server_fd);
        freeaddrinfo(server_info);
        throw std::runtime_error("Listen failed");
    }

    freeaddrinfo(server_info);
    return server_fd;
}

void handle_client(int client_fd)
{
    char buffer[Config::BUFFER_SIZE];

    {
        std::lock_guard<std::mutex> lock(logging_mutex);
        std::cout << "Client connected" << std::endl;
    }

    int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0)
    {
        std::lock_guard<std::mutex> lock(logging_mutex);
        std::cout << "Client disconnected" << std::endl;
        close(client_fd);
        return;
    }

    buffer[bytes_received] = '\0';
    std::string request(buffer);
    std::string response = handle_http_request(request);

    int bytes_sent = send(client_fd, response.c_str(), response.length(), 0);
    if (bytes_sent == -1)
    {
        std::lock_guard<std::mutex> lock(logging_mutex);
        std::cerr << "Send error" << std::endl;
    }

    close(client_fd);
    {
        std::lock_guard<std::mutex> lock(logging_mutex);
        std::cout << "Client connection closed" << std::endl;
    }
}
