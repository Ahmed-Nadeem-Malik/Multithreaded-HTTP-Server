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
#include <string_view>

#include "../include/config.h"
#include "../include/globals.h"
#include "../include/http.h"

void set_keep_alive(bool keep_alive);

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

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        close(server_fd);
        freeaddrinfo(server_info);
        throw std::runtime_error("setsockopt SO_REUSEADDR failed");
    }
#ifdef SO_REUSEPORT
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
    {
        close(server_fd);
        freeaddrinfo(server_info);
        throw std::runtime_error("setsockopt SO_REUSEPORT failed");
    }
#endif

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
    char read_chunk[Config::BUFFER_SIZE];

    if (Config::LOG_CONNECTIONS)
    {
        std::lock_guard<std::mutex> lock(logging_mutex);
        std::cout << "Client connected" << std::endl;
    }

    auto find_header_end = [](const char* data, int length) -> int
    {
        for (int i = 3; i < length; ++i)
        {
            if (data[i - 3] == '\r' && data[i - 2] == '\n' && data[i - 1] == '\r' && data[i] == '\n')
            {
                return i - 3;
            }
        }
        return -1;
    };

    auto parse_content_length = [](std::string_view headers, int& out_length) -> bool
    {
        static const std::string_view kHeader("Content-Length:");
        size_t pos = headers.find(kHeader);
        if (pos == std::string_view::npos)
        {
            return false;
        }

        pos += kHeader.size();
        while (pos < headers.size() && (headers[pos] == ' ' || headers[pos] == '\t'))
        {
            ++pos;
        }

        int value = 0;
        bool found_digit = false;
        while (pos < headers.size() && headers[pos] >= '0' && headers[pos] <= '9')
        {
            found_digit = true;
            value = value * 10 + (headers[pos] - '0');
            ++pos;
        }

        if (!found_digit)
        {
            return false;
        }

        out_length = value;
        return true;
    };

    auto parse_http_version = [](std::string_view headers) -> std::string_view
    {
        size_t line_end = headers.find("\r\n");
        if (line_end == std::string_view::npos)
        {
            return std::string_view();
        }
        std::string_view request_line = headers.substr(0, line_end);
        size_t last_space = request_line.rfind(' ');
        if (last_space == std::string_view::npos)
        {
            return std::string_view();
        }
        return request_line.substr(last_space + 1);
    };

    auto parse_connection_header = [](std::string_view headers) -> std::string_view
    {
        static const std::string_view kHeader("Connection:");
        size_t pos = headers.find(kHeader);
        if (pos == std::string_view::npos)
        {
            return std::string_view();
        }

        pos += kHeader.size();
        while (pos < headers.size() && (headers[pos] == ' ' || headers[pos] == '\t'))
        {
            ++pos;
        }

        size_t end = headers.find("\r\n", pos);
        if (end == std::string_view::npos)
        {
            end = headers.size();
        }
        return headers.substr(pos, end - pos);
    };

    auto send_all = [](int fd, const char* data, size_t length) -> bool
    {
        size_t total_sent = 0;
        while (total_sent < length)
        {
            ssize_t bytes_sent = send(fd, data + total_sent, length - total_sent, 0);
            if (bytes_sent <= 0)
            {
                return false;
            }
            total_sent += static_cast<size_t>(bytes_sent);
        }
        return true;
    };

    std::string buffer;
    buffer.reserve(Config::BUFFER_SIZE);
    size_t offset = 0;

    while (true)
    {
        if (buffer.size() - offset >= static_cast<size_t>(Config::BUFFER_SIZE - 1))
        {
            break;
        }

        if (offset > 0 && offset == buffer.size())
        {
            buffer.clear();
            offset = 0;
        }
        else if (offset > 4096)
        {
            buffer.erase(0, offset);
            offset = 0;
        }

        bool have_request = false;
        size_t request_length = 0;
        bool keep_alive = false;

        while (!have_request)
        {
            int available = static_cast<int>(buffer.size() - offset);
            int header_end = available > 0 ? find_header_end(buffer.data() + offset, available) : -1;
            if (header_end == -1)
            {
                ssize_t bytes_received = recv(client_fd, read_chunk, sizeof(read_chunk), 0);
                if (bytes_received <= 0)
                {
                    if (Config::LOG_CONNECTIONS)
                    {
                        std::lock_guard<std::mutex> lock(logging_mutex);
                        std::cout << "Client disconnected" << std::endl;
                    }
                    close(client_fd);
                    return;
                }
                buffer.append(read_chunk, static_cast<size_t>(bytes_received));
                if (buffer.size() - offset >= static_cast<size_t>(Config::BUFFER_SIZE - 1))
                {
                    close(client_fd);
                    return;
                }
                continue;
            }

            std::string_view headers(buffer.data() + offset, static_cast<size_t>(header_end));
            int content_length = 0;
            bool has_content_length = parse_content_length(headers, content_length);
            if (!has_content_length)
            {
                content_length = 0;
            }

            size_t total_needed = static_cast<size_t>(header_end) + 4 + static_cast<size_t>(content_length);
            if (static_cast<size_t>(available) < total_needed)
            {
                ssize_t bytes_received = recv(client_fd, read_chunk, sizeof(read_chunk), 0);
                if (bytes_received <= 0)
                {
                    if (Config::LOG_CONNECTIONS)
                    {
                        std::lock_guard<std::mutex> lock(logging_mutex);
                        std::cout << "Client disconnected" << std::endl;
                    }
                    close(client_fd);
                    return;
                }
                buffer.append(read_chunk, static_cast<size_t>(bytes_received));
                if (buffer.size() - offset >= static_cast<size_t>(Config::BUFFER_SIZE - 1))
                {
                    close(client_fd);
                    return;
                }
                continue;
            }

            request_length = total_needed;

            std::string_view http_version = parse_http_version(headers);
            std::string_view connection_value = parse_connection_header(headers);
            bool connection_close = connection_value.find("close") != std::string_view::npos;
            bool connection_keep = connection_value.find("keep-alive") != std::string_view::npos;

            if (http_version == "HTTP/1.0")
            {
                keep_alive = connection_keep;
            }
            else
            {
                keep_alive = !connection_close;
            }

            have_request = true;
        }

        set_keep_alive(keep_alive);
        std::string request = buffer.substr(offset, request_length);
        std::string response = handle_http_request(request);

        if (!send_all(client_fd, response.c_str(), response.size()))
        {
            std::lock_guard<std::mutex> lock(logging_mutex);
            std::cerr << "Send error" << std::endl;
            close(client_fd);
            return;
        }

        offset += request_length;
        if (!keep_alive)
        {
            break;
        }
    }

    close(client_fd);
    if (Config::LOG_CONNECTIONS)
    {
        std::lock_guard<std::mutex> lock(logging_mutex);
        std::cout << "Client connection closed" << std::endl;
    }
}
