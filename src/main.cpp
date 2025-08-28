// System includes
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// Standard library includes
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Constants
constexpr const char* PORT = "8080";
constexpr const char* SERVER_NAME = "MyHTTPServer/1.0";
constexpr int BUFFER_SIZE = 256;
constexpr int BACKLOG = 10;

// Utility functions
std::string read_file_content(const std::string& file_path)
{
    std::ifstream file(file_path);
    if (!file.is_open())
    {
        return "";
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string create_http_response(const std::string& status, const std::string& content_type, const std::string& content)
{
    return "HTTP/1.1 " + status +
           "\r\n"
           "Content-Length: " +
           std::to_string(content.size()) +
           "\r\n"
           "Server: " +
           std::string(SERVER_NAME) +
           "\r\n"
           "Content-Type: " +
           content_type +
           "\r\n"
           "\r\n" +
           content;
}

std::string create_file_response(const std::string& file_path, const std::string& content_type)
{
    std::string content = read_file_content(file_path);

    if (content.empty())
    {
        return create_http_response("404 Not Found", "text/html", "File not Found");
    }

    return create_http_response("200 OK", content_type, content);
}

std::string get_current_time()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t time_t = std::chrono::system_clock::to_time_t(now);
    std::string time_str = std::ctime(&time_t);
    time_str.pop_back();  // Remove newline
    return time_str;
}

std::string handle_http_request(const std::string& request)
{
    std::istringstream iss(request);
    std::string method, path;
    iss >> method >> path;

    if (method != "GET")
    {
        return create_http_response("405 Method Not Allowed", "text/html", "Method Not Allowed");
    }

    if (path == "/")
    {
        return create_file_response("../static/index.html", "text/html");
    }
    else if (path == "/hello")
    {
        return create_file_response("../static/hello.html", "text/html");
    }
    else if (path == "/time")
    {
        return create_http_response("200 OK", "text/html", get_current_time());
    }
    else if (path == "/css/style.css")
    {
        return create_file_response("../static/css/style.css", "text/css");
    }
    else if (path == "/js/script.js")
    {
        return create_file_response("../static/js/script.js", "application/javascript");
    }
    else
    {
        return create_http_response("404 Not Found", "text/html", "Not Found");
    }
}

void handle_client(int client_fd)
{
    char buffer[BUFFER_SIZE];

    std::cout << "Client connected" << std::endl;

    int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received == -1)
    {
        std::cerr << "Receive error" << std::endl;
        close(client_fd);
        return;
    }
    else if (bytes_received == 0)
    {
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
        std::cerr << "Send error" << std::endl;
    }
    else
    {
        std::cout << "Response sent to client" << std::endl;
    }

    close(client_fd);
    std::cout << "Client connection closed\n" << std::endl;
}

int setup_server_socket()
{
    struct addrinfo hints, *server_info;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(NULL, PORT, &hints, &server_info);
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

    if (listen(server_fd, BACKLOG) == -1)
    {
        close(server_fd);
        freeaddrinfo(server_info);
        throw std::runtime_error("Listen failed");
    }

    freeaddrinfo(server_info);
    return server_fd;
}

int main()
{
    try
    {
        int server_fd = setup_server_socket();
        std::cout << "Server listening on port " << PORT << std::endl;

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

            handle_client(client_fd);
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
