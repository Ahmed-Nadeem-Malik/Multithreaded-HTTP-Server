/**
 * @file server.cpp
 * @brief TCP server socket management and client connection handling
 */

#include "../include/server.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <mutex>
#include <stdexcept>

#include "../include/config.h"
#include "../include/globals.h"
#include "../include/http.h"

int setup_server_socket()
{
    // Configure socket address hints
    struct addrinfo hints, *server_info;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;        // IPv4 only
    hints.ai_socktype = SOCK_STREAM;  // TCP connection
    hints.ai_flags = AI_PASSIVE;      // Bind to any interface

    // Resolve server address and port
    int status = getaddrinfo(NULL, Config::PORT, &hints, &server_info);
    if (status != 0)
    {
        throw std::runtime_error("getaddrinfo failed: " + std::string(gai_strerror(status)));
    }

    // Create socket file descriptor
    int server_fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (server_fd == -1)
    {
        freeaddrinfo(server_info);
        throw std::runtime_error("Socket creation failed");
    }

    // Bind socket to address and port
    if (bind(server_fd, server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        close(server_fd);
        freeaddrinfo(server_info);
        throw std::runtime_error("Bind failed");
    }

    // Start listening for connections
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

    std::string request;
    int total_received = 0;

    // Read HTTP request data in chunks until complete
    while (total_received < Config::BUFFER_SIZE - 1)
    {
        int bytes_received = recv(client_fd, buffer + total_received, Config::BUFFER_SIZE - 1 - total_received, 0);

        if (bytes_received <= 0)
        {
            if (total_received == 0)
            {
                std::lock_guard<std::mutex> lock(logging_mutex);
                std::cout << "Client disconnected" << std::endl;
            }
            break;
        }

        total_received += bytes_received;
        buffer[total_received] = '\0';

        // Look for end of HTTP headers (double CRLF indicates end of headers)
        std::string partial(buffer, total_received);
        size_t header_end = partial.find("\r\n\r\n");

        if (header_end != std::string::npos)
        {
            // Headers complete, check if request has a body (POST/PUT requests)
            size_t content_length_pos = partial.find("Content-Length:");
            if (content_length_pos != std::string::npos)
            {
                // Extract the Content-Length header value to know how much body to expect
                size_t length_start = partial.find(":", content_length_pos) + 1;
                size_t length_end = partial.find("\r\n", length_start);
                std::string length_str = partial.substr(length_start, length_end - length_start);

                // Remove leading/trailing whitespace from header value
                length_str.erase(0, length_str.find_first_not_of(" \t"));

                int content_length = std::stoi(length_str);
                int expected_total = header_end + 4 + content_length;  // headers + "\r\n\r\n" + body

                if (total_received >= expected_total)
                {
                    break;  // Complete request with body received
                }
            }
            else
            {
                // No Content-Length header, assume no body (typical for GET requests)
                break;
            }
        }
    }

    // Process HTTP request and generate response
    request = std::string(buffer, total_received);
    std::string response = handle_http_request(request);

    // Send response back to client
    int bytes_sent = send(client_fd, response.c_str(), response.length(), 0);
    if (bytes_sent == -1)
    {
        std::lock_guard<std::mutex> lock(logging_mutex);
        std::cerr << "Send error" << std::endl;
    }

    // Close client connection
    close(client_fd);
    {
        std::lock_guard<std::mutex> lock(logging_mutex);
        std::cout << "Client connection closed" << std::endl;
    }
}
