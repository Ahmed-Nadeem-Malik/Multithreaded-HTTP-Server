#pragma once

#include <functional>
#include <string>
#include <unordered_map>

using Handler = std::function<std::string(const std::string& body)>;
using RouteMap = std::unordered_map<std::string, std::unordered_map<std::string, Handler>>;

// Routing functions
void add_route(const std::string& path, const std::string& method, Handler h);
Handler static_file(const std::string& file_path, const std::string& content_type);
RouteMap& get_routes();
void init_routes();