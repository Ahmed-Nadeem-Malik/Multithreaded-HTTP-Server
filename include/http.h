#pragma once

#include <string>

// HTTP utility functions
std::string read_file_content(const std::string& file_path);
std::string create_http_response(const std::string& status, const std::string& content_type, const std::string& content);
std::string create_file_response(const std::string& file_path, const std::string& content_type);
std::string get_current_time();
std::string handle_http_request(const std::string& request);