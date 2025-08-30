/**
 * @file config.h
 * @brief Server configuration constants
 */

#pragma once
namespace Config
{
constexpr const char* PORT = "8080";             // Server port
constexpr const char* SERVER_NAME = "MyHTTPServer/1.0";  // HTTP Server header
constexpr int BUFFER_SIZE = 2048;                // Client request buffer size
constexpr int BACKLOG = 10;                      // Socket listen queue size
constexpr int THREADS_NUM = 14;                  // Thread pool size
}  // namespace Config
