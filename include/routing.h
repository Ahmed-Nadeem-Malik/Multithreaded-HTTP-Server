#pragma once

#include <functional>
#include <string>
#include <unordered_map>

// HTTP request handler function type
using Handler = std::function<std::string(const std::string& body)>;
// Route mapping: path -> method -> handler
using RouteMap = std::unordered_map<std::string, std::unordered_map<std::string, Handler>>;

// Registers a route handler for given path and HTTP method
void add_route(const std::string& path, const std::string& method, Handler h);

// Creates handler that serves static files
Handler static_file(const std::string& file_path, const std::string& content_type);

// Returns reference to global route map
RouteMap& get_routes();

// Initialize all application routes
void init_routes();
