#pragma once

#include <string>

// Reads entire file into string, returns empty if file not found
std::string read_file_content(const std::string& file_path);

// Creates properly formatted HTTP response with headers
std::string create_http_response(const std::string& status, const std::string& content_type,
                                 const std::string& content);

// Creates HTTP response for static file serving
std::string create_file_response(const std::string& file_path, const std::string& content_type);

// Returns current system time as string
std::string get_current_time();

// Main HTTP request parser and router
std::string handle_http_request(const std::string& request);
