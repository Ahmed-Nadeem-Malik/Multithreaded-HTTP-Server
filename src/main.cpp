#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>

#define PORT "8080"

int main()
{
    // Socket setup variables
    int status;
    int server_fd;
    int client_fd;
    int bytes_received;
    int bytes_sent;
    const int backlog = 10;
    char buffer[255];

    // Address info structures
    struct addrinfo hints;
    struct addrinfo* server_info;
    struct sockaddr_storage client_addr;
    socklen_t addr_size;

    // Initialize hints structure
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;        // IPv4
    hints.ai_socktype = SOCK_STREAM;  // TCP
    hints.ai_flags = AI_PASSIVE;      // Use local IP

    // Get address info for the server
    if ((status = getaddrinfo(NULL, PORT, &hints, &server_info)) != 0)
    {
        throw std::runtime_error("getaddrinfo failed: " + std::string(gai_strerror(status)));
    }

    // Create socket
    server_fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (server_fd == -1)
    {
        freeaddrinfo(server_info);
        throw std::runtime_error("Socket creation failed");
    }

    // Bind socket to port
    if (bind(server_fd, server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        close(server_fd);
        freeaddrinfo(server_info);
        throw std::runtime_error("Bind failed");
    }

    // Start listening for connections
    if (listen(server_fd, backlog) == -1)
    {
        close(server_fd);
        freeaddrinfo(server_info);
        throw std::runtime_error("Listen failed");
    }

    std::cout << "Server listening on port " << PORT << std::endl;
    freeaddrinfo(server_info);  // No longer needed

    // Main server loop - handle clients one at a time
    while (true)
    {
        addr_size = sizeof(client_addr);

        // Accept incoming connection
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_size);
        if (client_fd == -1)
        {
            std::cerr << "Accept failed, continuing..." << std::endl;
            continue;
        }

        std::cout << "Client connected" << std::endl;

        // Receive data from client
        bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received == -1)
        {
            std::cerr << "Receive error" << std::endl;
        }
        else if (bytes_received == 0)
        {
            std::cout << "Client disconnected" << std::endl;
        }
        else
        {
            // Null-terminate and echo back
            buffer[bytes_received] = '\0';
            std::cout << "Received: " << buffer << std::endl;

            bytes_sent = send(client_fd, buffer, bytes_received, 0);
            if (bytes_sent == -1)
            {
                std::cerr << "Send error" << std::endl;
            }
            else
            {
                std::cout << "Echoed back to client" << std::endl;
            }
        }

        // Close client connection
        close(client_fd);
        std::cout << "Client connection closed\n" << std::endl;
    }

    // Cleanup (never reached due to infinite loop)
    close(server_fd);
    return 0;
}
