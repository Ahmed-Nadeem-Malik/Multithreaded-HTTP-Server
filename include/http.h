/**
 * @file http.h
 * @brief HTTP request handling and response utilities
 */

#pragma once

#include <string>

/**
 * Reads entire file contents into a string
 * @param file_path Path to the file to read
 * @return File contents as string, or empty string if file cannot be read
 */
std::string read_file_content(const std::string& file_path);

/**
 * Creates complete HTTP/1.1 response with headers
 * @param status HTTP status line (e.g., "200 OK", "404 Not Found")
 * @param content_type MIME type for Content-Type header
 * @param content Response body content
 * @return Complete HTTP response string
 */
std::string create_http_response(const std::string& status, const std::string& content_type,
                                 const std::string& content);

/**
 * Creates HTTP response with file content
 * @param file_path Path to file to serve
 * @param content_type MIME type for the file
 * @return HTTP response with file content, or 404 if file not found
 */
std::string create_file_response(const std::string& file_path, const std::string& content_type);

/**
 * Gets current system time as formatted string
 * @return Current time in ctime format (without trailing newline)
 */
std::string get_current_time();

/**
 * Main HTTP request processor that parses request and routes to handlers
 * @param request Raw HTTP request string
 * @return HTTP response based on routing
 */
std::string handle_http_request(const std::string& request);
