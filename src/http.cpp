#include "../include/http.h"

#include <chrono>
#include <fstream>
#include <sstream>

#include "../include/config.h"
#include "../include/globals.h"
#include "../include/routing.h"

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
    return "HTTP/1.1 " + status + "\r\nContent-Length: " + std::to_string(content.size()) +
           "\r\nServer: " + std::string(Config::SERVER_NAME) + "\r\nContent-Type: " + content_type + "\r\n\r\n" +
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
    time_str.pop_back();  // Remove trailing newline
    return time_str;
}

std::string handle_http_request(const std::string& request)
{
    ++request_counter;

    std::istringstream iss(request);
    std::string method, path, version;
    iss >> method >> path >> version;

    // Extract body (after blank line)
    std::string body;
    size_t sep = request.find("\r\n\r\n");
    if (sep != std::string::npos)
    {
        body = request.substr(sep + 4);
    }

    if (method.empty() || path.empty())
    {
        return create_http_response("400 Bad Request", "text/plain", "Bad Request");
    }

    auto route = get_routes().find(path);
    if (route == get_routes().end())
    {
        return create_http_response("404 Not Found", "text/plain", "Not Found");
    }

    auto handler = route->second.find(method);
    if (handler == route->second.end())
    {
        return create_http_response("405 Method Not Allowed", "text/plain", "Method Not Allowed");
    }

    return handler->second(body);
}
