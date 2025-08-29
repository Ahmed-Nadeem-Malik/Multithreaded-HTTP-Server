#include "../include/server.h"

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

#include "../include/config.h"
#include "../include/http.h"
#include "../include/globals.h"

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

    std::string request;
    int total_received = 0;
    
    // Read until we have headers + body
    while (total_received < Config::BUFFER_SIZE - 1) {
        int bytes_received = recv(client_fd, buffer + total_received, 
                                Config::BUFFER_SIZE - 1 - total_received, 0);
        
        if (bytes_received <= 0) {
            if (total_received == 0) {
                std::lock_guard<std::mutex> lock(logging_mutex);
                std::cout << "Client disconnected" << std::endl;
            }
            break;
        }
        
        total_received += bytes_received;
        buffer[total_received] = '\0';
        
        // Check if we have complete HTTP request (headers + body)
        std::string partial(buffer, total_received);
        size_t header_end = partial.find("\r\n\r\n");
        
        if (header_end != std::string::npos) {
            // Found end of headers, check if we need more data for body
            size_t content_length_pos = partial.find("Content-Length:");
            if (content_length_pos != std::string::npos) {
                // Extract content length
                size_t length_start = partial.find(":", content_length_pos) + 1;
                size_t length_end = partial.find("\r\n", length_start);
                std::string length_str = partial.substr(length_start, length_end - length_start);
                
                // Remove leading whitespace
                length_str.erase(0, length_str.find_first_not_of(" \t"));
                
                int content_length = std::stoi(length_str);
                int expected_total = header_end + 4 + content_length;
                
                if (total_received >= expected_total) {
                    // We have complete request
                    break;
                }
            } else {
                // No content-length, assume complete
                break;
            }
        }
    }
    
    request = std::string(buffer, total_received);
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
